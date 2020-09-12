#include <distrac/analysis/event.hpp>
#include <distrac/analysis/event_definition.hpp>
#include <distrac/analysis/node.hpp>
#include <distrac/analysis/property.hpp>
#include <distrac/analysis/property_definition.hpp>

#include <cassert>

namespace distrac {
event::event(const event_definition& def,
             const distrac::node& node,
             uint64_t number)
  : _def(def)
  , _node(node)
  , _number(number) {
  init_properties();
}
event::event(const event& ev) = default;

event::~event() {}

void
event::init_properties() {
  _properties.clear();
  std::size_t offset = 0;
  for(auto& definition : _def.definitions()) {
    _properties.push_back(distrac::property(*this, definition, offset));
    offset += definition.size();
  }
}

bool
event::valid(uint64_t number) const {
  return number < _node.event_count(_def.id());
}

int64_t
event::timestamp_with_offset() const {
  assert(valid());
  return timestamp() + _node.offset_ns();
}

uint8_t
event::id() const {
  return _def.id();
}

distrac_id
event::node_id() const {
  return _node.id();
}

const uint8_t*
event::memory(uint64_t number) const {
  return _node.get_event_memory(_def.id(), number);
}

const property&
event::property(size_t id) const {
  return _properties.at(id);
};
}