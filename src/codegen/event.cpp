#include <distrac/codegen/event.hpp>
#include <distrac/codegen/event_definition.hpp>
#include <distrac/codegen/property.hpp>
#include <distrac/codegen/property_definition.hpp>

namespace distrac {
event::event(event_definition& def, uint8_t* src)
  : _def(def)
  , _src(src) {
  init_properties();
}

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

const property&
event::property(size_t id) const {
  return _properties.at(id);
};
}
