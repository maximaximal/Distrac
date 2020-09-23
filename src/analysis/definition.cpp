#include "distrac/types.h"
#include <distrac/analysis/definition.hpp>
#include <distrac/analysis/event_definition.hpp>
#include <distrac/analysis/property_definition.hpp>

#include <boost/algorithm/string.hpp>

#include <iostream>

namespace distrac {
definition::definition() {}
definition::definition(const std::string& name, const std::string& description)
  : _name(name)
  , _description(description) {}
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
    cout << "    Event " << ev.name() << " (" << ev.description()
         << ") :" << endl;
    for(auto& prop : ev.definitions()) {
      cout << "      Property " << prop.name() << " : " << prop.type() << endl;
    }
  }
}

void
definition::generate_c_header(std::ostream& o) const {
  using std::endl;

  std::string include_guard = "DISTRAC_GENERATED_HEADER_" + prefix();

  // clang-format off
  // This is hard to understand if it underwent clang formatting.

  o << "#ifndef " << include_guard << endl;
  o << "#define " << include_guard << endl;
  o << endl;
  o << "/* THIS FILE WAS AUTOMATICALLY GFNERATED USING DISTRAC!" << endl;
  o << " * BE AWARE OF POSSIBLE ISSUES WHEN EDITING AND RE-GENERATING! */" << endl;
  o << endl;
  o << "#ifdef __cplusplus" << endl;
  o << "extern \"C\" {" << endl;
  o << "#endif" << endl;
  o << endl;
  o << "#ifdef DISTRAC_DEFINITION" << endl;
  o << endl;
  o << "distrac_event_header " << prefix() << "event_headers[" << _definitions.size() << "] = {" << endl;
  for(const auto &ev : _definitions) {
    o << "  distrac_event_header{\"" << ev.name() << "\", \"" << ev.description() << "\", " << ev.property_count() << " }," << endl;
  }
  o << "};" << endl;
  o << endl;
  for(const auto &ev : _definitions) {
    o << "distrac_property_header *" << prefix() << "ev" << std::to_string(ev.id()) << "_property_headers[" << ev.property_count() << "] = {" << endl;
    for(const auto &prop : ev.definitions()) {
      std::string type = distrac_type_to_str(prop.type());
      boost::to_upper(type);
      type = "DISTRAC_TYPE_" + type;
      o << "  distrac_property_header{ \"" << prop.name() << "\", " << type << ", false, 0, 0 }," << endl;
    }
    o << "};" << endl;
  }
  o << endl;
  o << "distrac_property_header " << prefix() << "property_headers[" << _definitions.size() << "] = {" << endl;
  for(const auto &ev : _definitions) {
    o << "  " << prefix() << "ev" << std::to_string(ev.id()) << "_property_headers," << endl;
  }
  o << "};" << endl;
  o << endl;
  o << "void " << prefix() << "distrac_definition(distrac_definition *def) {" << endl;
  o << "  def->file_header.event_count = " << _definitions.size() << ";" << endl;
  o << "  def->event_headers = " << prefix() << "event_headers;" << endl;
  o << "  def->property_headers = " << prefix() << "property_headers;" << endl;
  o << "}" << endl;
  o << endl;
  o << "#else" << endl;
  o << endl;
  o << "extern distrac_event_header *" << prefix() << "event_headers;" << endl;
  for(const auto &ev : _definitions) {
    o << "extern distrac_property_header *" << prefix() << "ev" << std::to_string(ev.id()) << "_property_headers;" << endl;
  }
  o << "extern distrac_property_header *" << prefix() << "property_headers;" << endl;
  o << "void " << prefix() << "distrac_definition(distrac_definition *def);" << endl;
  o << endl;
  o << "#endif" << endl;
  o << endl;
  o << "#ifdef __cplusplus" << endl;
  o << "}" << endl;
  o << "#endif" << endl;
  o << endl;
  o << "#endif // " << include_guard << endl;

  // clang-format on
}
}
