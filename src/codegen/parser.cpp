#include "distrac/headers.h"
#include "distrac/types.h"

#include <boost/fusion/adapted.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/spirit/home/qi/string/symbols.hpp>
#include <boost/spirit/home/support/common_terminals.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_list.hpp>

#include <boost/algorithm/string/trim.hpp>

#include <distrac/codegen/parser.hpp>

#include <distrac/analysis/definition.hpp>
#include <distrac/analysis/event_definition.hpp>
#include <distrac/analysis/property_definition.hpp>

#include <limits>
#include <map>

using attributes_map = std::map<std::string, std::string>;

namespace distrac {
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

namespace parser_def {
struct property {
  std::string name;
  distrac_type type;
  attributes_map attributes;
};
struct event {
  std::string name;
  std::string description;
  std::vector<property> properties;
  attributes_map attributes;
};
struct definition {
  attributes_map attributes;
  std::vector<event> events;
};
}
}

// clang-format off
BOOST_FUSION_ADAPT_STRUCT(distrac::parser_def::property,
                          (std::string, name),
                          (distrac_type, type),
                          (attributes_map, attributes))

BOOST_FUSION_ADAPT_STRUCT(distrac::parser_def::event,
                          (std::string, name),
                          (std::string, description),
                          (std::vector<distrac::parser_def::property>, properties),
                          (attributes_map, attributes))

BOOST_FUSION_ADAPT_STRUCT(distrac::parser_def::definition,
                          (attributes_map, attributes),
                          (std::vector<distrac::parser_def::event>, events))
// clang-format on

namespace distrac {
template<typename Iterator, typename Skipper>
struct event_definition_parser
  : qi::grammar<Iterator, parser_def::definition(), Skipper> {

  event_definition_parser()
    : event_definition_parser::base_type(def, "definition") {
    using ascii::alnum;
    using ascii::alpha;
    using ascii::char_;
    using boost::phoenix::construct;
    using boost::phoenix::val;
    using qi::double_;
    using qi::fail;
    using qi::int_;
    using qi::lexeme;
    using qi::lit;
    using qi::no_case;
    using namespace boost::spirit::qi;

    quoted_string %= lexeme['"' >> +(char_ - '"') >> '"'];
    identifier %= lexeme[alpha >> +(alnum | char_('_'))];
    text %= lexeme[*(char_ - ';' - '{')];

    qi::symbols<char, distrac_type> types;
    for(size_t i = 0; i < DISTRAC_TYPE__COUNT; ++i) {
      distrac_type t = static_cast<distrac_type>(i);
      types.add(distrac_type_to_str(t), t);
    }

    attribute %= identifier > '=' > quoted_string;

    property %=
      identifier >> ':' >> no_case[types] >> -('[' > attribute % ',' > ']');

    event %= identifier >> ((':' > text > '{') | '{') >> +property >> '}' >>
             -('[' > +attribute > ']');

    def %= (attribute % ';') >> +event;

    quoted_string.name("quoted string");
    text.name("unquoted string");
    identifier.name("identifier");
    attribute.name("attribute");
    property.name("property");
    event.name("event");
    def.name("definition");

    on_error<fail>(
      def,
      std::cerr << val("!! Parsing Error! Expecting ") << _4// what failed?
                << val(" here: \"")
                << construct<std::string>(_3, _2)// iterators to error-pos, end
                << val("\"") << std::endl);
  }

  qi::rule<Iterator, std::string(), Skipper> identifier;
  qi::rule<Iterator, std::string(), Skipper> text;
  qi::rule<Iterator, std::pair<std::string, std::string>, Skipper> attribute;
  qi::rule<Iterator, parser_def::property(), Skipper> property;
  qi::rule<Iterator, parser_def::event(), Skipper> event;
  qi::rule<Iterator, parser_def::definition(), Skipper> def;
  qi::rule<Iterator, std::string(), Skipper> quoted_string;
};

parser::parser() {}
parser::~parser() {}

parser::result
parser::generate_definition(parser_def::definition& def) {
  using std::cerr;
  using std::endl;

  std::string desc = "(no description)";
  if(!def.attributes.count("description")) {
    cerr << "Warn: No `description` defined for definition. Defaulting to no "
            "(no description)."
         << endl;
  } else {
    desc = def.attributes["description"];
    boost::algorithm::trim_right(desc);
    if(desc.size() > DISTRAC_DESCRIPTION_LENGTH) {
      return parser_error{ "Definition \"" + desc +
                           "\" has a too long description size!" };
    }
  }

  std::string name = "unnamed";
  if(!def.attributes.count("name")) {
    cerr << "Warn: No `name` defined for definition. Defaulting to no "
            "unnamed"
         << endl;
  } else {
    name = def.attributes["name"];
    if(name.size() > DISTRAC_NAME_LENGTH) {
      return parser_error{ "Definition \"" + name + "\" has a too long name!" };
    }
  }

  definition out_def(name, desc);

  if(!def.attributes.count("prefix")) {
    cerr << "Warn: No `prefix` defined for definition. Defaulting to no prefix."
         << endl;
  }
  else {
    out_def.set_prefix(def.attributes["prefix"]);
  }

  out_def.reserve(def.events.size());

  uint8_t ev_id = 0;
  for(auto& ev : def.events) {
    if(out_def.get_event_id(ev.name) != -1) {
      return parser_error{ "Event \"" + ev.name +
                           "\" was already defined previously!" };
    }
    if(ev.name.size() > DISTRAC_NAME_LENGTH) {
      return parser_error{ "Event \"" + ev.name + "\" has a too long name!" };
    }
    boost::algorithm::trim_right(ev.description);
    if(ev.description.size() > DISTRAC_DESCRIPTION_LENGTH) {
      return parser_error{ "Event \"" + ev.name +
                           "\" has a too long description!" };
    }

    event_definition ev_def(ev.name, ev_id++, ev.description);
    uint8_t prop_id = 0;
    for(const auto& prop : ev.properties) {
      if(prop.name.size() > DISTRAC_NAME_LENGTH) {
        return parser_error{ "Property \"" + prop.name + "\" in event \"" +
                             ev.name + "\" has a too long name!" };
      }
      if(prop_id == std::numeric_limits<uint8_t>::max()) {
        return parser_error{
          "Event \"" + ev.name +
          "\" has too many properties! Could not add property \"" + prop.name +
          "\". Maximum number is 255."
        };
      }
      property_definition prop_def(prop.name, prop.type, prop_id++);
      if(ev_def.get_property_id(prop.name) != -1) {
        return parser_error{ "Property \"" + prop.name +
                             "\" was already defined previously in event \"" +
                             ev.name + "\"!" };
      }
      if(!prop_def.is_correctly_aligned(ev_def)) {
        return parser_error{ "Property \"" + prop.name + "\" in event \"" +
                             ev.name +
                             "\" is not properly aligned for its type!" };
      }
      ev_def.add_property_definition(prop_def);
    }
    if((ev_def.size() & 0b00000111) != 0) {
      return parser_error{ "Event \"" + ev.name +
                           "\" must have a size that is multiple of 8 bytes!" };
    }
    out_def.add_definition(ev_def);
  }

  return out_def;
}

parser::result
parser::parse_definition(const std::string& definition) {
  using Iterator = std::string::const_iterator;
  using Skipper = qi::rule<Iterator>;

  // Skipper for comments from https://stackoverflow.com/a/44531686
  Skipper block_comment, single_line_comment, skipper;
  {
    using namespace qi;
    single_line_comment = "//" >> *(char_ - eol) >> (eol | eoi);
    block_comment = ("/*" >> *((block_comment | char_) - "*/")) > "*/";

    skipper = space | single_line_comment | block_comment;
  }

  using distrac_grammar = event_definition_parser<Iterator, Skipper>;
  distrac_grammar definition_grammar;
  parser_def::definition def;

  std::string::const_iterator iter = definition.begin();
  std::string::const_iterator end = definition.end();

  bool r = phrase_parse(iter, end, definition_grammar, skipper, def);

  if(r && iter == end) {
    return generate_definition(def);
  } else {
    std::string::const_iterator some = iter + 30;
    std::string context(iter, (some > end) ? end : some);
    std::string error = "Parsing failed! Stopped at \"" + context + "\"";
    return parser_error{ error };
  }
}
}
