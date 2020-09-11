#pragma once

#include <cstddef>
#include <string>

#include <distrac/headers.h>
#include <distrac/types.h>

namespace distrac {
class property_definition {
  public:
  property_definition(const std::string& name, distrac_type type)
    : _name(name)
    , _type(type) {}
  property_definition(const distrac_property_header& header)
    : _name(header.name)
    , _type(static_cast<distrac_type>(header.datatype)) {}
  ~property_definition() {}

  const std::string& name() const { return _name; }
  distrac_type type() const { return _type; }
  std::size_t size() const { return distrac_type_sizeof(_type); }

  private:
  std::string _name;
  distrac_type _type;
};
}
