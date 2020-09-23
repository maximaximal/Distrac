#pragma once

#include <cstddef>
#include <map>
#include <string>
#include <vector>

#include <distrac/headers.h>

namespace distrac {

class property_definition;

class event_definition {
  public:
  using property_definition_vector = std::vector<property_definition>;
  using property_definition_map = std::map<std::string, property_definition&>;

  event_definition(const std::string& name,
                   uint8_t id,
                   const std::string& description = "");
  event_definition(const distrac_event_header& header, uint8_t id);
  event_definition(const event_definition &o);
  ~event_definition();

  std::size_t property_size(std::size_t id) const;
  std::size_t property_count() const { return _defs.size(); }
  const property_definition_vector& definitions() const { return _defs; }

  const property_definition& definition(std::size_t id) const {
    return _defs.at(id);
  };

  void add_property_definition(const property_definition& def);

  const std::string& name() const { return _name; }
  const std::string& description() const { return _description; }
  std::size_t size() const { return _size; }
  uint8_t id() const { return _id; }

  ssize_t get_property_id(const std::string& name) const;

  bool has_causal_dependency() const { return _has_causal_dependency; }
  void set_has_causal_dependency(bool v) { _has_causal_dependency = v; }

  uint8_t causal_dependency_event_id() const {
    return _causal_dependency_event_id;
  }
  void set_causal_dependency_event_id(uint8_t id) {
    _causal_dependency_event_id = id;
  }

  private:
  void recompute_map();

  const std::string _name;
  property_definition_vector _defs;
  property_definition_map _defs_map;
  std::size_t _size = 0;
  const uint8_t _id;
  const std::string _description;

  bool _has_causal_dependency = false;
  uint8_t _causal_dependency_event_id = 0;
};
}
