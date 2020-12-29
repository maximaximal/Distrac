#include "distrac/analysis/event_definition.hpp"
#include "distrac/analysis/property_definition.hpp"
#include "distrac/analysis/tracefile.hpp"
#include "distrac/headers.h"
#include <distrac/analysis/event.hpp>
#include <distrac/analysis/node.hpp>

#include <distrac/analysis/event_iterator.hpp>

namespace distrac {
node::node(distrac_node_header& header,
           const tracefile& trace,
           size_t tracefile_location_index)
  : _header(header)
  , _id(header.node_id)
  , _tracefile_location_index(tracefile_location_index)
  , _name(header.node_name)
  , _hostname(header.node_hostname)
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
node::~node() = default;

size_t
node::compute_following_size() {
  size_t size = sizeof(uint64_t) * _trace.event_definitions().size();
  for(size_t ev = 0; ev < _trace.event_definitions().size(); ++ev) {
    _event_memory_locations.push_back(
      reinterpret_cast<const uint8_t*>(&_header) + sizeof(_header) + size);
    size += (_trace.event_definitions()[ev].size() + sizeof(int64_t)) *
            _event_counts[ev];
  }
  return size;
}

const uint8_t*
node::get_event_memory(uint8_t event, uint64_t number) const {
  assert(event < _event_memory_locations.size());
  if(number > _event_counts[event]) {
    return nullptr;
  }

  return _event_memory_locations[event] +
         number * (_trace.event_definitions()[event].size() + sizeof(int64_t));
  // The size of the timestamp must be counted aswell.
}

event_iterator
node::begin() const {
  event_iterator it;

  for(auto& def : _trace.event_definitions()) {
    it.add_event(event(def, *this));
  }
  return it;
}

event_iterator
node::end() const {
  return event_iterator();
}

int64_t
node::offset() const {
  return _header.offset_ns;
}
void
node::set_offset(int64_t offset) {
  _header.offset_ns = offset;
}

event
node::get_event(uint8_t id, uint64_t number) const {
  assert(id < _trace.event_definitions().size());
  return event(_trace.event_definitions()[id], *this, number);
}
}
