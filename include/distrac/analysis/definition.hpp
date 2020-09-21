#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace distrac {
class event_definition;

class definition {
  public:
  using event_definition_vector = std::vector<distrac::event_definition>;
  using event_definition_map =
    std::map<std::string, distrac::event_definition&>;

  definition(const std::string& description);
  ~definition();

  const std::string& description() const { return _description; }

  const event_definition_vector& definitions() const { return _definitions; }
  ssize_t get_event_id(const std::string& name) const;

  void reserve(uint8_t count);
  void add_definition(const event_definition& def);

  void print_summary() const;

  private:
  const std::string _description;
  event_definition_vector _definitions;
  event_definition_map _definitions_map;
};
}
