#include "distrac/distrac.h"
#include "distrac/headers.h"
#include "distrac/types.h"

#include <chrono>
#include <distrac/analysis/definition.hpp>
#include <distrac/analysis/entry_matcher.hpp>
#include <distrac/analysis/event_definition.hpp>
#include <distrac/analysis/event_iterator.hpp>
#include <distrac/analysis/property_definition.hpp>
#include <distrac/analysis/tracefile.hpp>
#include <distrac/analysis/util.hpp>

#include <forward_list>
#include <future>
#include <iomanip>
#include <iostream>

using std::cerr;
using std::clog;
using std::cout;
using std::endl;

namespace distrac {
tracefile::tracefile(const std::string& path)
  : _path(path)
  , _definition("unnamed", "Parsed from tracefile " + path) {
  _sink.open(path);
  if(!_sink.is_open()) {
    std::cerr << "!! Could not open file as memory mapped file!" << std::endl;
    exit(EXIT_FAILURE);
  }

  _byte_size = _sink.size();

  if(_byte_size < sizeof(distrac_file_header)) {
    std::cerr << "!! Trace file too small!" << std::endl;
    exit(EXIT_FAILURE);
  }

  scan();
  if(requiresOffsetRecalculation()) {
    calculate_offsets();
  }
  calculate_beginAndEndTime();
}
tracefile::~tracefile() = default;

void
tracefile::print_summary() {
  cout << "DISTRAC TRACE SUMMARY" << endl;
  cout << "  Problem Name: " << _header->problem_name << endl;
  cout << "  Additional Info: " << _header->additional_info << endl;
  cout << "  Metadata: " << _header->metadata << endl;
  cout << "  DisTrac Definition Name: " << _header->distrac_name << endl;
  cout << "  DisTrac Definition Description: " << _header->distrac_description
       << endl;
  {
    auto time = trace_time();
    cout << "  Start Time: " << std::put_time(std::localtime(&time), "%F %T%z")
         << endl;
  }
  cout << "  Events:" << endl;
  for(auto& ev : event_definitions()) {
    cout << "    Event " << ev.name() << " (encountered "
         << event_count(ev.id()) << "x):" << endl;
    for(auto& prop : ev.definitions()) {
      if(prop.name()[0] == '_')
        continue;
      cout << "      Property " << prop.name() << " : " << prop.type() << endl;
    }
  }
  cout << "  Nodes:" << endl;
  for(auto& node : _nodes) {
    cout << "    Node " << node.name() << " (Hostname: " << node.hostname()
         << "):" << endl;
    cout << "      Program: " << node.program() << endl;
    cout << "      Internal Node Number: " << node.tracefile_location_index()
         << endl;
    cout << "      Offset NS: " << node.offset() << endl;
    cout << "      ID: " << node.id() << endl;
    for(size_t ev = 0; ev < event_definitions().size(); ++ev) {
      cout << "      " << node.event_count(ev) << " "
           << event_definitions()[ev].name() << " events" << endl;
    }
  }
}

const tracefile::event_definition_vector&
tracefile::event_definitions() const {
  return _definition.definitions();
}
ssize_t
tracefile::get_event_id(const std::string& name) const {
  return _definition.get_event_id(name);
}

void
tracefile::scan() {
  // Step 1: Sweep over whole trace and collect required data.

  size_t pos = 0;

  _header = reinterpret_cast<distrac_file_header*>(&_sink.data()[pos]);
  pos += sizeof(distrac_file_header);

  if(_header->distrac_trace_file_signature_bytes != DISTRAC_FILE_SIGNATURE) {
    std::cerr
      << "!! File does not have distrac file signature bytes at the beginning!"
      << std::endl;

    exit(EXIT_FAILURE);
  }

  _definition.set_name(_header->distrac_name);
  _definition.set_description(_header->distrac_description);

  _definition.reserve(_header->event_count);

  // Parse event and property headers.
  for(uint8_t ev = 0; ev < _header->event_count; ++ev) {
    const auto& ev_header = read_struct<distrac_event_header>(pos);

    event_definition ev_definition{ ev_header, ev };

    for(uint8_t prop = 1; prop < ev_header.property_count + 1; ++prop) {
      const auto& prop_header = read_struct<distrac_property_header>(pos);
      ev_definition.add_property_definition(
        property_definition{ prop_header, prop });
    }

    _definition.add_definition(ev_definition);
  }

  // Parse nodes and event data.
  while(is_size_left(pos, sizeof(distrac_node_header))) {
    read_until_aligned(pos);
    auto& node_header = read_struct<distrac_node_header>(pos);

    node n{ node_header, *this, _nodes.size() };

    pos += n.following_size();

    _nodes.push_back(std::move(n));
  }
}

void
tracefile::calculate_offsets() {
  std::vector<entrymatcher> matchers;
  std::vector<std::future<entrymatcher::offsetvector>> futures;

  for(const auto& ev_def : _definition.definitions()) {
    if(ev_def.has_causal_dependency()) {
      matchers.emplace_back(_definition, ev_def, _nodes);
      futures.push_back(std::async(
        std::launch::async, std::bind(&entrymatcher::run, matchers.back())));
    }
  }

  entrymatcher::offsetvector offsets(_nodes.size());

  for(auto& f : futures) {
    auto o = f.get();
    for(std::size_t i = 0; i < o.size(); ++i) {
      offsets[i] = std::min(o[i], offsets[i]);
    }
  }

  for(std::size_t i = 0; i < _nodes.size(); ++i) {
    _nodes[i].set_offset(offsets[i]);
  }

  _header->number_of_nodes_with_aligned_offsets = _nodes.size();
}

void
tracefile::calculate_beginAndEndTime() {
  int64_t begin_ns = 0;
  int64_t end_ns = 0;

  for(auto& node : _nodes) {
    for(uint8_t ev_id = 0; ev_id < _definition.get_event_count(); ++ev_id) {
      if(node.event_count(ev_id) == 0)
        continue;

      const auto first_event = node.get_event(ev_id, 0);
      const auto last_event =
        node.get_event(ev_id, node.event_count(ev_id) - 1);
      begin_ns = std::min(begin_ns, first_event.timestamp_with_offset());
      end_ns = std::max(end_ns, last_event.timestamp_with_offset());
    }
  }

  // Convert into time points.
  _begin_time =
    time_point(std::chrono::seconds(_header->seconds_since_epoch_on_start) +
               std::chrono::nanoseconds(begin_ns));
  _end_time =
    time_point(std::chrono::seconds(_header->seconds_since_epoch_on_start) +
               std::chrono::nanoseconds(end_ns));
}

bool
tracefile::requiresOffsetRecalculation() {
  return _nodes.size() != _header->number_of_nodes_with_aligned_offsets;
}

void
tracefile::assert_size_left(size_t pos, size_t size, const char* structname) {
  if(!is_size_left(pos, size)) {
    std::cerr << "!! Cannot read " << size << " bytes for " << structname
              << "! Current pos is " << pos << " total size is " << _byte_size
              << ". Exiting." << std::endl;
    exit(EXIT_FAILURE);
  }
}

size_t
tracefile::event_count(uint8_t ev) const {
  size_t count = 0;
  for(const auto& node : _nodes) {
    count += node.event_count(ev);
  }
  return count;
}

event_iterator
tracefile::begin() const {
  event_iterator it;

  std::forward_list<event> filtered_events;

  for(const auto& node : _nodes) {
    auto events = node.begin().events();
    std::copy(
      events.begin(), events.end(), std::front_inserter(filtered_events));
  }

  it.add_events(std::begin(filtered_events), std::end(filtered_events));

  return it;
}

event_iterator
tracefile::end() const {
  return event_iterator();
}

tracefile::filtered_tracefile
tracefile::filtered(event_filter_func func) const {
  return filtered_tracefile{ std::move(func), *this };
}
tracefile::filtered_tracefile
tracefile::filtered(const event_filter_set& event_ids) const {
  return filtered_tracefile{
    [event_ids](const event& ev) { return event_ids.count(ev.id()); }, *this
  };
}

event_iterator
tracefile::filtered_tracefile::begin() const {
  event_iterator it;

  std::forward_list<event> filtered_events;

  for(const auto& node : trace._nodes) {
    auto events = node.begin().events();
    std::copy_if(
      events.begin(), events.end(), std::front_inserter(filtered_events), func);
  }

  it.add_events(std::begin(filtered_events), std::end(filtered_events));

  return it;
}

event_iterator
tracefile::filtered_tracefile::end() const {
  return event_iterator();
}

}
