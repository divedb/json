#pragma once

#include <cerrno>
#include <climits>
#include <cstdlib>

#include "json/json_value.h"
#include "json/pipe.h"
#include "json/util.h"

namespace json {

inline constexpr auto is_zero = is_byte<'0'>;
inline constexpr auto is_dot_pipe = PipeOne(is_byte<'.'>);
inline constexpr auto is_opt_minus_pipe = PipeZeroOrOne(is_byte<'-'>);
inline constexpr auto is_opt_plus_pipe = PipeZeroOrOne(is_byte<'+'>);
inline constexpr auto is_exponent_pipe = PipeOne(is_exponent);

// int           = zero ｜ ( digit1-9 *DIGIT )
template <typename InputIt>
constexpr bool parse_int(ParserState<InputIt>& state) {
  if (state | is_digit_pipe; !state.is_ok()) {
    return false;
  }

  u8 b = state.back();

  if (is_zero(b)) {
    IF_EOF_RETURN(state, true);

    // 0 can't be followed by other digits.
    if (state | is_non_digit_pipe; state.is_ok()) {
      state.put(state.pop_back());

      return true;
    }

    return false;
  }

  state | is_zero_or_more_digits_pipe;

  return state.is_ok();
}

// decimal-point = %x2E                         ; .
// frac          = decimal-point 1*DIGIT
template <typename InputIt>
constexpr bool parse_frac(ParserState<InputIt>& state) {
  state.succeed_pipes = 0;
  state | is_dot_pipe | is_digit_pipe | is_zero_or_more_digits_pipe;

  return state.is_ok();
}

// e             = %x65/%x45                    ; e-E
// exp           = e [ minus | plus ] 1*DIGIT   ;
template <typename InputIt>
constexpr bool parse_exponent(ParserState<InputIt>& state) {
  state.succeed_pipes = 0;
  state | is_exponent_pipe | is_opt_minus_pipe | is_opt_plus_pipe |
      is_digit_pipe | is_zero_or_more_digits_pipe;

  return state.is_ok();
}

// Numeric values that cannot be represented in the grammar below (such
// as Infinity and NaN) are not permitted.
// number = [ minus ] int [ frac ] [ exp ]
//
// decimal-point = %x2E                         ; .
// digit1-9      = %x31-39                      ; 1-9
// e             = %x65/%x45                    ; e-E
// exp           = e [ minus | plus ] 1*DIGIT   ;
// frac          = decimal-point 1*DIGIT
// int           = zero / ( digit1-9 *DIGIT )
// minus         = %x2D                         ; -
// plus          = %x2B                         ; +
// zero          = %x30                         ; 0
template <typename InputIt>
constexpr bool parse_number_aux(ParserState<InputIt>& state, bool& is_float) {
  state | is_opt_minus_pipe;

  if (!parse_int(state)) {
    return false;
  }

  IF_EOF_RETURN(state, true);

  bool has_frac = false;
  bool has_exponent = false;

  // Parse optional frac part.
  // Here, succeed_pipes > 0 means the state passes through at least 1 pipe.
  // That is `dot pipe`.
  if (has_frac = parse_frac(state); !has_frac && state.succeed_pipes > 0) {
    return false;
  }

  is_float = is_float || has_frac;
  state.status = Status::kSucceed;

  // Parse optional exponent part.
  if (has_exponent = parse_exponent(state);
      !has_exponent && state.succeed_pipes > 0) {
    return false;
  }

  is_float = is_float || has_exponent;

  return true;
}

// float       strtof( const char* str, char** str_end );
// double      strtod( const char* str, char** str_end );
// long double strtold( const char* str, char** str_end );
//
// Interprets a floating point value in a byte string pointed to by str.
// Function discards any whitespace characters (as determined by std::isspace)
// until first non-whitespace character is found. Then it takes as many
// characters as possible to form a valid floating-point representation and
// converts them to a floating-point value. The valid floating-point value
// can be one of the following:
template <typename InputIt>
JsonValue parse_number(ParserState<InputIt>& state) {
  bool is_float;
  Buffer buf;
  JsonValue json_value;

  if (parse_number_aux(state, is_float)) {
    buf = state.buffer();

    if (is_float) {
      auto res = std::strtold(buf.c_str(), nullptr);

      if (errno == ERANGE && res == HUGE_VALL) {
        state.error = std::make_shared<NumberError>(buf + " OVERFLOW!");
      } else {
        json_value = Number{res};
      }
    } else {
      auto res = std::strtoll(buf.c_str(), nullptr, 10);

      if (res == LLONG_MIN && errno == ERANGE) {
        state.error = std::make_shared<NumberError>(buf + " UNDERFLOW!");
      } else if (res == LLONG_MAX && errno == ERANGE) {
        state.error = std::make_shared<NumberError>(buf + " OVERFLOW!");
      } else {
        json_value = Number{res};
      }
    }
  } else {
    if (state.status == Status::kEOF) {
      state.error = std::make_shared<NumberError>(buf + " EOF!");
    } else {
      Buffer error("Unknown byte ");
      error.push_back(state.next());
      state.error = std::make_shared<NumberError>(error);
    }
  }

  return json_value;
}

}  // namespace json