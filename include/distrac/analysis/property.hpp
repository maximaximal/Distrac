#pragma once

#include <distrac/types.h>

namespace distrac {
class property_definition;
class event;

class property {
  public:
  property(const distrac::event& event,
           const property_definition& def,
           std::size_t offset);
  ~property();

  const property_definition& definition() const { return _def; }
  const distrac::event& event() const { return _event; }

  distrac_type type() const;
  std::size_t offset() const { return _offset; };

  const uint8_t* memory() const;

  private:
  const distrac::event& _event;
  const property_definition& _def;
  std::size_t _offset;
};
}
