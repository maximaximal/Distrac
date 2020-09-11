#include "distrac/analysis/event_definition.hpp"
#include "distrac/analysis/property_definition.hpp"
#include "distrac/analysis/tracefile.hpp"
#include "distrac/headers.h"
#include <distrac/analysis/node.hpp>

namespace distrac {
node::node(const distrac_node_header& header, const tracefile& trace)
  : _header(header)
  , _id(header.node_id)
  , _name(header.node_name)
  , _program(header.program_name)
  , _distrac_verion(header.distrac_version)
  , _offset_ns(header.offset_ns)
  , _trace(trace) {
  _event_counts.resize(trace.event_definitions().size());

  const uint8_t* event_counts_arr_pos =
    (reinterpret_cast<const uint8_t*>(&header) + sizeof(distrac_node_header));
  memcpy(_event_counts.data(),
         event_counts_arr_pos,
         sizeof(uint64_t) * trace.event_definitions().size());
  _following_size = compute_following_size();
}
node::~node() {}

size_t
node::compute_following_size() {
  size_t size = 0;
  for(size_t ev = 0; ev < _trace.event_definitions().size(); ++ev) {
    size += _trace.event_definitions()[ev].size() * _event_counts[ev];
    _event_offsets.push_back(reinterpret_cast<const uint8_t*>(&_header) + size);
  }
  return size;
}
}
