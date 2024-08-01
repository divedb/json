#pragma once

#include <memory>

#include "json/common/constants.hpp"
#include "json/nodes/json_array.hpp"
#include "json/parser/number_parser.hpp"
#include "json/parser/parser_common.hpp"
#include "json/parser/string_parser.hpp"

namespace json {

/// An array structure is represented as square brackets surrounding zero or
/// more values (or elements). Elements are separated by commas.
/// array = begin-array [ value *( value-separator value ) ] end-array
/// There is no requirement that the values in an array be of the same type.
template <typename InputIt, typename Allocator = std::allocator<JsonArray>>
InputIt parse_array(InputIt first, InputIt last, JsonValue& json_value,
                    ErrorCode& err, Allocator& alloc = Allocator{}) {
  char ch = *first++;

  assert(ch == kOpenBracket);

  // Keep track the level of '['.
  int depth = 0;
  err = ErrorCode::kOk;

  while (err == ErrorCode::kOk) {
    CHECK_EOF(first, last, err);

    ch = *first++;

    if (ch == kQuote) {
      JsonValue str;
      first = parse_string(first, last, str, err);
      json_value.as_array()->append(str);
    }
  }

  return first;
}

}  // namespace json