#include "json/parser/json_parser.hpp"

namespace json {

JsonNode* Parser::parse(CharStream& stream, ParserState& state) {
  if (stream.is_eof()) {
    state.set_error("Reach end of file while attempting to read a character");

    return nullptr;
  }

  char ch = stream.next_char();

  
}

}  // namespace json