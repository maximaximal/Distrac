#include <distrac/codegen/event_definition.hpp>
#include <distrac/codegen/property_definition.hpp>

namespace distrac {
event_definition::event_definition(const std::string& name)
  : _name(name) {}
event_definition::~event_definition() {}

std::size_t
event_definition::property_size(std::size_t id) const {
  const property_definition& def = definition(id);
  return def.size();
}

void
event_definition::add_property_definition(const property_definition& def) {
  _defs.push_back(def);
  _size += def.size();
}
}
