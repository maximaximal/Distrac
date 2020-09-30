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

  definition();
  definition(std::string name, std::string description);
  ~definition();

  const event_definition_vector& definitions() const { return _definitions; }
  ssize_t get_event_id(const std::string& name) const;

  void reserve(uint8_t count);
  void add_definition(const event_definition& def);

  void print_summary() const;

  void generate_c_header(std::ostream& o) const;

  void set_name(const std::string& name) { _name = name; };
  const std::string& name() const { return _name; }

  void set_description(const std::string& description) {
    _description = description;
  };
  const std::string& description() const { return _description; }

  void set_prefix(const std::string& prefix) { _prefix = prefix; };
  const std::string& prefix() const { return _prefix; }

  private:
  std::string _name;
  std::string _description;
  std::string _prefix;
  event_definition_vector _definitions;
  event_definition_map _definitions_map;
};
}
