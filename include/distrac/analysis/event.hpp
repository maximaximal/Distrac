#pragma once

#include <distrac/types.h>

#include <cstdint>
#include <vector>

namespace distrac {
class node;
class property;
class event_definition;

class event {
  public:
  event(const event_definition& def,
        const distrac::node& node,
        uint64_t number = 0);
  event(const event& ev);
  ~event();

  void init_properties();

  uint64_t number() const { return _number; }

  bool valid() const { return valid(_number); };
  bool valid(uint64_t number) const;
  uint8_t id() const;
  const std::string& ev_name() const;
  const distrac_id& node_id() const;
  size_t node_tracefile_location_index() const;

  void set_number(uint64_t number) const { _number = number; }

  int64_t timestamp() const {
    if(valid())
      return *reinterpret_cast<const int64_t*>(memory());
    else
      return 0;
  };
  int64_t timestamp_with_offset() const;

  const ::distrac::property& property(size_t id) const;

  const uint8_t* properties_memory(uint64_t number) const {
    if(valid(number))
      return memory(number) + sizeof(timestamp());
    else
      return nullptr;
  };
  const uint8_t* properties_memory() const {
    return properties_memory(_number);
  };

  const ::distrac::node& node() const { return _node; }
  const uint8_t* memory() const { return memory(_number); }
  const uint8_t* memory(uint64_t number) const;

  bool operator<(const event& rhs) const {
    return id() < rhs.id() && node_id() < rhs.node_id();
  }
  void operator++() const { ++_number; }
  bool operator==(const event& rhs) const {
    return _number == rhs._number && id() == rhs.id() &&
           node_id() == rhs.node_id() &&
           node_tracefile_location_index() ==
             rhs.node_tracefile_location_index();
  }

  std::ostream& csv_header_out(std::ostream &o) const;
  std::ostream& csv_out(std::ostream &o) const;

  private:
  const event_definition& _def;
  const ::distrac::node& _node;

  mutable uint64_t _number;

  std::vector<::distrac::property> _properties;
};
}
