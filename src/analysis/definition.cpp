#include <distrac/analysis/definition.hpp>
#include <distrac/analysis/event_definition.hpp>
#include <distrac/analysis/property_definition.hpp>

#include <iostream>

namespace distrac {
definition::definition(const std::string& description)
  : _description(description) {}
definition::~definition() {}

void
definition::reserve(uint8_t count) {
  _definitions.reserve(count);
}

void
definition::add_definition(const event_definition& def) {
  _definitions.push_back(def);
  _definitions_map.insert(
    { def.name(), _definitions[_definitions.size() - 1] });
}

ssize_t
definition::get_event_id(const std::string& name) const {
  const auto& it = _definitions_map.find(name);
  if(it == _definitions_map.end())
    return -1;
  return it->second.id();
}

void
definition::print_summary() const {
  using std::cout;
  using std::endl;
  cout << "  Events:" << endl;
  for(auto& ev : _definitions) {
    cout << "    Event " << ev.name() << " (" << ev.description() << ") :" << endl;
    for(auto& prop : ev.definitions()) {
      cout << "      Property " << prop.name() << " : " << prop.type() << endl;
    }
  }
}
}
