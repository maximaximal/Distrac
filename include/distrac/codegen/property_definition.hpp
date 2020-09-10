#pragma once

#include <cstddef>
#include <string>

#include <distrac/types.h>

namespace distrac {
class property_definition {
  public:
  property_definition(const std::string& name, distrac_type type)
    : _name(name)
    , _type(type) {}
  ~property_definition() {}

  const std::string& name() const { return _name; }
  distrac_type type() const { return _type; }
  std::size_t size() const { return distrac_type_sizeof(_type); }

  private:
  std::string _name;
  distrac_type _type;
};
}
