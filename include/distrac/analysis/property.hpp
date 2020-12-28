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
  const distrac::event& event() const {
    assert(_event != nullptr);
    return *_event;
  }
  void set_event(const distrac::event& ev) { _event = &ev; }

  const std::string& name() const;
  distrac_type type() const;
  std::size_t offset() const { return _offset; };
  std::size_t size() const;

  const uint8_t* memory() const;

  private:
  const distrac::event* _event;
  const property_definition& _def;
  std::size_t _offset;
};

std::ostream&
operator<<(std::ostream& o, const property& p);
}
