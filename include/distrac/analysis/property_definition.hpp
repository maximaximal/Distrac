#pragma once

#include <cstddef>
#include <string>

#include <distrac/headers.h>
#include <distrac/types.h>

namespace distrac {
class property_definition {
  public:
  property_definition(const std::string& name, distrac_type type, uint8_t id)
    : _name(name)
    , _type(type)
    , _id(id) {}
  property_definition(const distrac_property_header& header, uint8_t id)
    : _name(header.name)
    , _type(static_cast<distrac_type>(header.datatype))
    , _id(id) {}
  ~property_definition() {}

  const std::string& name() const { return _name; }
  distrac_type type() const { return _type; }
  std::size_t size() const { return distrac_type_sizeof(_type); }
  uint8_t id() const { return _id; }

  private:
  std::string _name;
  distrac_type _type;
  uint8_t _id;
};
}
