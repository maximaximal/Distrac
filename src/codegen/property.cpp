#include "distrac/codegen/property_definition.hpp"
#include <distrac/codegen/event.hpp>
#include <distrac/codegen/property.hpp>

namespace distrac {
property::property(const distrac::event& event,
                   const property_definition& def,
                   std::size_t offset)
  : _event(event)
  , _def(def)
  , _offset(offset) {}
property::~property() {}

distrac_type
property::type() const {
  return _def.type();
}

const uint8_t*
property::memory() const {
  return _event.memory() + _offset;
}
}
