#include "distrac/headers.h"
#include <distrac/analysis/event_definition.hpp>
#include <distrac/analysis/property_definition.hpp>

#include <distrac-analysis_export.h>

namespace distrac {
DISTRAC_ANALYSIS_EXPORT
event_definition::event_definition(const std::string& name,
                                   uint8_t id,
                                   const std::string& description)
  : _name(name)
  , _id(id)
  , _description(description) {}
DISTRAC_ANALYSIS_EXPORT
event_definition::event_definition(const distrac_event_header& header,
                                   uint8_t id)
  : _name(header.name)
  , _id(id)
  , _description(header.description) {
  _defs.reserve(header.property_count);
}
DISTRAC_ANALYSIS_EXPORT
event_definition::event_definition(const event_definition& o)
  : _name(o._name)
  , _defs(o._defs)
  , _size(o._size)
  , _id(o._id)
  , _description(o._description)
  , _has_causal_dependency(o._has_causal_dependency)
  , _causal_dependency_event_id(o._causal_dependency_event_id) {
  recompute_map();
}
DISTRAC_ANALYSIS_EXPORT event_definition::~event_definition() {}

DISTRAC_ANALYSIS_EXPORT std::size_t
event_definition::property_size(std::size_t id) const {
  const property_definition& def = definition(id);
  return def.size();
}

DISTRAC_ANALYSIS_EXPORT void
event_definition::add_property_definition(const property_definition& def) {
  _defs.push_back(def);
  _size += def.size();
  _defs_map.insert({ def.name(), _defs[_defs.size() - 1] });
}

ssize_t
event_definition::get_property_id(const std::string& name) const {
  const auto& it = _defs_map.find(name);
  if(it == _defs_map.end())
    return -1;
  return it->second.id();
}

void
event_definition::recompute_map() {
  for(size_t i = 0; i < _defs.size(); ++i) {
    _defs_map.insert({ _defs[i].name(), _defs[i] });
  }
}
}
