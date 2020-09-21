#pragma once

#include <variant>

namespace distrac {
class definition;

namespace parser_def {
class property;
class event;
class definition;
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
  result generate_definition(const parser_def::definition& def);
};
}
