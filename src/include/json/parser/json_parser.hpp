#pragma once

#include <cctype>
#include <memory>

#include "json/common/error.hpp"
#include "json/nodes/json_node.hpp"

namespace json {

struct Location {
  void update(int ch) {
    if (std::isspace(ch)) {
      y_pos++;
      x_pos = 0;
    } else {
      x_pos++;
    }

    source_location++;
  }

  int x_pos{};
  int y_pos{};
  int source_location{};
};

class ParserState {
 public:
  constexpr bool is_ok() const { return is_ok_; }

  constexpr void set_error(Error err) {
    err_ = err;
    is_ok_ = false;
  }

 private:
  bool is_ok_{true};
  Error err_;
};

class CharStream;
class MemoryContext;

class Parser {
 public:
  explicit Parser(MemoryContext& mem_ctx) : mem_ctx_{mem_ctx} {}

  JsonNode* parse(CharStream& stream);

 private:
  void parse_null(CharStream& stream, ParserState& state, JsonNode*& jnode);
  void parse_bool(CharStream& stream, ParserState& state, JsonNode*& jnode);
  void parse_number(CharStream& stream, ParserState& state, JsonNode*& jnode);

  JsonNode* parse_string(CharStream& stream, ParserState& state);
  JsonNode* parse_array(CharStream& stream, ParserState& state);
  JsonNode* parse_object(CharStream& stream, ParserState& state);

  MemoryContext& mem_ctx_;
};

}  // namespace json