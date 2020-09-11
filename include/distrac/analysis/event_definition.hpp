#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include <distrac/headers.h>

namespace distrac {

class property_definition;

class event_definition {
  public:
  using property_definition_vector = std::vector<property_definition>;

  event_definition(const std::string& name);
  event_definition(const distrac_event_header& header);
  ~event_definition();

  std::size_t property_size(std::size_t id) const;
  std::size_t property_count() const { return _defs.size(); }
  const property_definition_vector& definitions() const { return _defs; }

  const property_definition& definition(std::size_t id) const {
    return _defs.at(id);
  };

  void add_property_definition(const property_definition& def);

  const std::string& name() const { return _name; }
  std::size_t size() const { return _size; }

  private:
  property_definition_vector _defs;
  std::string _name;
  std::size_t _size = 0;
};
}
