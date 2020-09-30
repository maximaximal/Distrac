#pragma once

#include <variant>

namespace distrac {
class definition;

namespace parser_def {
struct property;
struct event;
struct definition;
}

class parser {
  public:
  struct parser_error {
    std::string msg;
  };
  using result = std::variant<definition, parser_error>;

  parser();
  ~parser();

  result parse_definition(const std::string& definition);

  private:
  static result generate_definition(parser_def::definition& def);
};
}
