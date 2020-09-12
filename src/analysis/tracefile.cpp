#include "distrac/distrac.h"
#include "distrac/headers.h"
#include "distrac/types.h"

#include <distrac/analysis/event_definition.hpp>
#include <distrac/analysis/property_definition.hpp>
#include <distrac/analysis/tracefile.hpp>
#include <distrac/analysis/util.hpp>
#include <distrac/analysis/event_iterator.hpp>

#include <iostream>

using std::cerr;
using std::clog;
using std::cout;
using std::endl;

namespace distrac {
tracefile::tracefile(const std::string& path)
  : _path(path) {
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
  cout << "  Events:" << endl;
  for(auto& ev : _event_definitions) {
    cout << "    Event " << ev.name() << ":" << endl;
    for(auto& prop : ev.definitions()) {
      cout << "      Property " << prop.name() << " : " << prop.type() << endl;
    }
  }
  cout << "  Nodes:" << endl;
  for(auto& node : _nodes) {
    cout << "    Node " << node.name() << ":" << endl;
    cout << "      Program: " << node.program() << endl;
    cout << "      Offset: " << node.offset_ns() << endl;
    for(uint8_t ev = 0; ev < _event_definitions.size(); ++ev) {
      cout << "      " << node.event_count(ev) << " "
           << _event_definitions[ev].name() << " events" << endl;
    }
    cout << "      Events:" << endl;
    for(auto &ev : node) {
      cout << "Ev: " << ev.number() << " at " << ev.timestamp_with_offset() << " of type " << (int)ev.id() << endl;
    }
  }
}

void
tracefile::scan() {
  // Step 1: Sweep over whole trace and collect required data.

  size_t pos = 0;

  _header = reinterpret_cast<distrac_file_header*>(&_sink.data()[pos]);
  pos += sizeof(distrac_file_header);

  if(_header->distrac_trace_file_signature_bytes != DISTRAC_FILE_SIGNATURE) {
    std::cerr
      << "!! File does not have distrac signature bytes at the beginning!"
      << std::endl;
    exit(EXIT_FAILURE);
  }

  _event_definitions.reserve(_header->event_count);

  // Parse event and property headers.
  for(uint8_t ev = 0; ev < _header->event_count; ++ev) {
    const auto& ev_header = read_struct<distrac_event_header>(pos);

    event_definition ev_definition{ ev_header, ev };

    for(uint8_t prop = 0; prop < ev_header.property_count; ++prop) {
      const auto& prop_header = read_struct<distrac_property_header>(pos);
      ev_definition.add_property_definition(property_definition{ prop_header, prop });
    }

    _event_definitions.push_back(std::move(ev_definition));
  }

  // Parse nodes and event data.
  while(is_size_left(pos, sizeof(distrac_node_header))) {
    read_until_aligned(pos);
    const distrac_node_header& node_header =
      read_struct<distrac_node_header>(pos);

    node n{ node_header, *this };

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
}
