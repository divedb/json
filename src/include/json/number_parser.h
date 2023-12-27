#pragma once

#include "json/parse_state.h"

namespace json {

// int           = zero / ( digit1-9 *DIGIT )
template <typename InputIter>
bool parse_int(InputIter& first, InputIter last, ParseState& ps) {
  bool has_zero = has_one(first, last, is_zero, ps);

  if (ps.state == State::kEOF) {
    return false;
  }

  bool has_digit = has_one_or_more(first, last, is_digit, ps);

  // case 1: other digits follows 0
  // case 2: no digits
  if ((has_zero && has_digit) || (!has_zero && !has_digit)) {
    ps.state = State::kInvalidNumber;

    return false;
  }

  return true;
}

// Numeric values that cannot be represented in the grammar below (such
// as Infinity and NaN) are not permitted.
// number = [ minus ] int [ frac ] [ exp ]
//
// decimal-point = %x2E                         ; .
// digit1-9      = %x31-39                      ; 1-9
// e             = %x65/%x45                    ; e-E
// exp           = e [ minus / plus ] 1*DIGIT   ;
// frac          = decimal-point 1*DIGIT
// int           = zero / ( digit1-9 *DIGIT )
// minus         = %x2D                         ; -
// plus          = %x2B                         ; +
// zero          = %x30                         ; 0
template <typename InputIter>
bool parse_number(InputIter& first, InputIter last, ParseState& ps,
                  JsonValue& out) {
  using VT = typename std::iterator_traits<InputIter>::value_type;

  InputIter init = first;

  bool has_minus = has_one(first, last, is_minus, ps);

  if (!parse_int(first, last, ps)) {
    return false;
  }

  bool has_frac = has_one(first, last, is_decimal_point, ps);

  if (has_frac && !has_one_or_more(first, last, is_digit, ps)) {
    return false;
  }

  bool has_exp = has_one(first, last, is_exp, ps);
  auto is_minus_or_plus = [](VT v) { return is_minus(v) || is_plus(v); };

  if (has_exp && (!has_one(first, last, is_minus_or_plus, ps) ||
                  !has_one_or_more(first, last, is_digit, ps))) {
    return false;
  }

  if (!has_frac) {
    BigInteger num = strtol(init, (char**)NULL, 10);
    out = JsonValue{Number(num)};
  } else {
    double v = strtod(init, NULL);
    out = JsonValue{Number(v)};
  }

  return true;
}

}  // namespace json