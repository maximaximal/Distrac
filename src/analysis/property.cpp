#include "distrac/analysis/property_definition.hpp"
#include "distrac/types.h"
#include <distrac/analysis/event.hpp>
#include <distrac/analysis/property.hpp>

namespace distrac {
property::property(const distrac::event& event,
                   const property_definition& def,
                   std::size_t offset)
  : _event(&event)
  , _def(def)
  , _offset(offset) {}
property::~property() = default;

const std::string&
property::name() const {
  return _def.name();
}

distrac_type
property::type() const {
  return _def.type();
}

const uint8_t*
property::memory() const {
  return _event->properties_memory() + _offset;
}

std::ostream&
operator<<(std::ostream& o, const property& p) {
  distrac_memory_to_type(p.memory(), p.type(), [&o](auto&& val) { o << val; });
  return o;
}
}
