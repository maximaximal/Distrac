#pragma once

#include <cstdint>
#include <vector>

namespace distrac {
class property;
class event_definition;

class event {
  public:
  event(event_definition& def, uint8_t* src = nullptr);
  ~event();

  void init_properties();

  bool valid() const { return _src != nullptr; };

  int64_t timestamp() const {
    if(valid())
      return *reinterpret_cast<int64_t*>(_src);
    else
      return 0;
  };
  const ::distrac::property& property(size_t id) const;

  const uint8_t* memory() const {
    if(valid())
      return _src + sizeof(timestamp());
    else
      return nullptr;
  };

  private:
  event_definition& _def;
  uint8_t* _src;

  std::vector<::distrac::property> _properties;
};
}
