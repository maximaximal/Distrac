#include "distrac/distrac.h"
#include "distrac/headers.h"
#include "distrac/types.h"

#include <distrac/analysis/definition.hpp>
#include <distrac/analysis/event_definition.hpp>
#include <distrac/analysis/event_iterator.hpp>
#include <distrac/analysis/property_definition.hpp>
#include <distrac/analysis/tracefile.hpp>
#include <distrac/analysis/util.hpp>

#include <forward_list>
#include <iomanip>
#include <iostream>

using std::cerr;
using std::clog;
using std::cout;
using std::endl;

namespace distrac {
tracefile::tracefile(const std::string& path)
  : _path(path)
  , _definition("Parsed from tracefile " + path) {
  _sink.open(path);
  if(!_sink.is_open()) {
    std::cerr << "!! Could not open file as memory mapped file!" << std::endl;
    exit(EXIT_FAILURE);
  }

  _byte_size = _sink.size();

  std::clog << "-> Opened trace of size " << BytePrettyPrint(_byte_size)
            << std::endl;

  if(_byte_size < sizeof(distrac_file_header)) {
    std::cerr << "!! Trace file too small!" << std::endl;
    exit(EXIT_FAILURE);
  }

  scan();
}
tracefile::~tracefile() {}

void
tracefile::print_summary() {
  cout << "DISTRAC TRACE SUMMARY" << endl;
  cout << "  Problem Name: " << _header->problem_name << endl;
  cout << "  Additional Info: " << _header->additional_info << endl;
  cout << "  Metadata: " << _header->metadata << endl;
  cout << "  DisTrac Description: " << _header->distrac_description << endl;
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
      cout << "      Property " << prop.name() << " : " << prop.type() << endl;
    }
  }
  cout << "  Nodes:" << endl;
  for(auto& node : _nodes) {
    cout << "    Node " << node.name() << " (Hostname: " << node.hostname()
         << "):" << endl;
    cout << "      Program: " << node.program() << endl;
    cout << "      Offset: " << node.offset_ns() << endl;
    cout << "      ID: " << node.id() << endl;
    for(uint8_t ev = 0; ev < event_definitions().size(); ++ev) {
      cout << "      " << node.event_count(ev) << " "
           << event_definitions()[ev].name() << " events" << endl;
    }
  }
}

const tracefile::event_definition_vector&
tracefile::event_definitions() const {
  return _definition.definitions();
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

  _definition.reserve(_header->event_count);

  // Parse event and property headers.
  for(uint8_t ev = 0; ev < _header->event_count; ++ev) {
    const auto& ev_header = read_struct<distrac_event_header>(pos);

    event_definition ev_definition{ ev_header, ev };

    for(uint8_t prop = 0; prop < ev_header.property_count; ++prop) {
      const auto& prop_header = read_struct<distrac_property_header>(pos);
      ev_definition.add_property_definition(
        property_definition{ prop_header, prop });
    }

    _definition.add_definition(ev_definition);
  }

  // Parse nodes and event data.
  while(is_size_left(pos, sizeof(distrac_node_header))) {
    read_until_aligned(pos);
    const distrac_node_header& node_header =
      read_struct<distrac_node_header>(pos);

    node n{ node_header, *this, _nodes.size() };

    pos += n.following_size();

    _nodes.push_back(std::move(n));
  }
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

  for(auto& node : _nodes) {
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
  return filtered_tracefile{ func, *this };
}
tracefile::filtered_tracefile
tracefile::filtered(std::set<uint8_t> event_ids) const {
  return filtered_tracefile{
    [event_ids](const event& ev) { return event_ids.count(ev.id()); }, *this
  };
}

event_iterator
tracefile::filtered_tracefile::begin() const {
  event_iterator it;

  std::forward_list<event> filtered_events;

  for(auto& node : trace._nodes) {
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
