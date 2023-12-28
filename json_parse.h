#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iterator>
#include <string>

namespace json {

static inline bool is_exp(int v) { return v == 'e' || v == 'E'; }
static inline bool is_decimal_point(int v) { return v == '.'; }
static inline bool is_minus(int v) { return v == '-'; }
static inline bool is_plus(int v) { return v == '+'; }
static inline bool is_digit(int v) { return v >= '0' && v <= '9'; }
static inline bool is_zero(int v) { return v == '0'; }

ParseState parse_number(const char* input, JsonValue& json_value) {
  ParseState ps;
  std::size_t size = strlen(input);

  if (!parse_number(input, input + size, ps, json_value)) {
    ps.state = State::kInvalidNumber;
  } else {
    ps.state = State::kOk;
  }

  return ps;
}

}  // namespace json