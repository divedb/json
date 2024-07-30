#include "json/parser/json_parser.hpp"

#include <string.h>  // strncmp

#include <charconv>  // from_chars
#include <vector>

#include "json/common/constants.hpp"
#include "json/common/memory_context.hpp"
#include "json/parser/char_stream.hpp"

namespace json {

#define CHECK_EOF(stream, state, stage)               \
  do {                                                \
    if (stream.is_eof()) {                            \
      state.set_error(Error{stage, ErrorCode::kEOF}); \
      return;                                         \
    }                                                 \
    while (0)

JsonNode* Parser::parse(CharStream& stream) {
  ParserState state;
  JsonNode* jnode = nullptr;

  while (!stream.is_eof() && state.is_ok()) {
    char ch = stream.peek();

    std::cout << "ch = " << ch << std::endl;

    switch (ch) {
      case 'n':
        parse_null(stream, state, jnode);
        break;

      case 'f':
      case 't':
        parse_bool(stream, state, jnode);
        break;

      case '-':
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':

      default:
        assert(0);
    }
  }

  return jnode;
}

/// @brief null = %x6e.75.6c.6c   ; null
void Parser::parse_null(CharStream& stream, ParserState& state,
                        JsonNode*& jnode) {
  const int n = 4;
  std::vector<char> nchars = stream.next_nchars(n);

  if (nchars.size() == n && strncmp(nchars.data(), kNullValue, n) == 0) {
    jnode = mem_ctx_.create<JsonNode>(JsonValue::null());

    return;
  }

  state.set_error(ParserState::Error::kParseNull);
}

/// @brief false = %x66.61.6c.73.65   ; false
///        true  = %x74.72.75.65      ; true
void Parser::parse_bool(CharStream& stream, ParserState& state,
                        JsonNode*& jnode) {
  char ch = stream.peek();
  char const* expect = kFalseValue;

  if (ch == 't') {
    expect = kTrueValue;
  }

  int n = strlen(expect);
  std::vector<char> nchars = stream.next_nchars(n);

  if (nchars.size() == n && strncmp(expect, nchars.data(), n) == 0) {
    bool b = (ch == 't');
    JsonValue json_value(b);
    jnode = mem_ctx_.create<JsonNode>(json_value);

    return;
  }

  state.set_error(ParserState::Error::kParseBool);
}

void Parser::parse_number(CharStream& stream, ParserState& state,
                          JsonNode*& jnode) {}

}  // namespace json