#include "distrac/headers.h"
#include "distrac/types.h"
#include <distrac/analysis/event_definition.hpp>
#include <distrac/analysis/property_definition.hpp>

namespace distrac {
event_definition::event_definition(std::string name,
                                   uint8_t id,
                                   std::string description)
  : _name(std::move(name))
  , _id(id)
  , _description(std::move(description)) {
  add_property_definition({ "_id", DISTRAC_TYPE_UINT64, 0 });
}
event_definition::event_definition(const distrac_event_header& header,
                                   uint8_t id)
  : _name(header.name)
  , _id(id)
  , _description(header.description)
  , _has_causal_dependency(header.enable_after_constraint)
  , _causal_dependency_event_id(header.after_event_id) {
  _defs.reserve(header.property_count + 1);
  add_property_definition({ "_id", DISTRAC_TYPE_UINT64, 0 });
}
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
event_definition::~event_definition() = default;

std::size_t
event_definition::property_size(std::size_t id) const {
  const property_definition& def = definition(id);
  return def.size();
}

void
event_definition::add_property_definition(const property_definition& def) {
  _defs.push_back(def);
  if(def.name()[0] != '_') {
    _size += def.size();
  }
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
  // NOLINTNEXTLINE modernize-loop-convert
  for(size_t i = 0; i < _defs.size(); ++i) {
    _defs_map.insert({ _defs[i].name(), _defs[i] });
  }
}
}
