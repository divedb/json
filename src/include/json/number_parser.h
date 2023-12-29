#pragma once

#include "json/parser_state.h"

namespace json {

inline auto is_zero = is_byte<'0'>;

// int           = zero ｜ ( digit1-9 *DIGIT )
template <typename InputIt>
constexpr bool parse_int(ParserState<InputIt>& state) {
  IF_EOF_RETURN(state, false);

  if (state = state | is_digit_pipe; !state.is_ok()) {
    return false;
  }

  byte b = state.back();

  if (is_zero(b)) {
    IF_EOF_RETURN(state, true);

    // Digits can't follow after leading 0.
    if (state = state | is_digit_pipe; state.is_ok()) {
      return false;
    }

    state.put(state.pop_back());

    return true;
  }

  state = state | is_zero_or_more_digits_pipe;

  return true;
}

// frac          = decimal-point 1*DIGIT
template <typename InputIt>
constexpr bool parse_frac(ParserState<InputIt>& state) {
  state.pipes = 0;
  state = state | is_dot_pipe | is_digit_pipe | is_zero_or_more_digits_pipe;

  return state.is_ok();
}

// e             = %x65/%x45                    ; e-E
// exp           = e [ minus | plus ] 1*DIGIT   ;
template <typename InputIt>
constexpr bool parse_exponent(ParserState<InputIt>& state) {
  state.pipes = 0;
  state = state | is_exponent_pipe | is_opt_minus_pipe | is_opt_plus_pipe |
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
constexpr bool parse_number(ParserState<InputIt>& state) {
  state = state | is_opt_minus_pipe;

  if (!parse_int(state)) {
    return false;
  }

  IF_EOF_RETURN(state, true);

  // Parse optional frac part.
  // Here, pipes > 0 means the state passes through at least 1 pipe.
  // That is `dot pipe`.
  if (!parse_frac(state) && state.pipes > 0) {
    return false;
  }

  // Parse optional exponent part.
  if (!parse_exponent(state) && state.pipes > 0) {
    return false;
  }

  return true;
}

}  // namespace json