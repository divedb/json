#pragma once

#include "json/parser_state.h"

namespace json {

inline constexpr auto is_zero = is_byte<'0'>;
inline constexpr auto is_minus = is_byte<'-'>;
inline auto is_exp_pipe = make_fixed_pipe(is_exponent, 1);
inline constexpr auto is_dot_pipe = make_fixed_pipe(is_byte<'.'>, 1);
inline constexpr auto is_optional_minus = make_optional_pipe(is_byte<'-'>, 1);
inline constexpr auto is_optional_plus = make_optional_pipe(is_byte<'+'>, 1);

// int           = zero ｜ ( digit1-9 *DIGIT )
template <typename InputIt>
constexpr bool parse_int(ParserState<InputIt>& state, Buffer& out_buf) {
  IF_EOF_RETURN(state, false);

  u8 b = state.next();

  if (!is_digit(b)) {
    return false;
  }

  out_buf.push_back(b);

  if (is_zero(b)) {
    IF_EOF_RETURN(state, true);

    // TODO(gc): what next byte will be? [e|E|.|,]
    if (b = state.next(); is_digit(b)) {
      return false;
    }

    state.put(b);

    return true;
  }

  state = state | is_0_or_more_digits_pipe;
  out_buf.append(is_0_or_more_digits_pipe.buffer());

  return true;
}

// frac          = decimal-point 1*DIGIT
template <typename InputIt>
constexpr bool parse_frac(ParserState<InputIt>& state, Buffer& out_buf) {
  if (state = state | is_dot_pipe | is_1_or_more_digits_pipe; !state.is_ok()) {
    return false;
  }

  out_buf.append(is_dot_pipe.buffer());
  out_buf.append(is_1_or_more_digits_pipe.buffer());

  return true;
}

// e             = %x65/%x45                    ; e-E
// exp           = e [ minus | plus ] 1*DIGIT   ;
template <typename InputIt>
constexpr bool parse_exponent(ParserState<InputIt>& state, Buffer& out_buf) {
  if (!state = state | is_exp_pipe | is_optional_minus | is_optional_plus |
               is_1_or_more_digits_pipe;
      state.is_ok()) {
    return false;
  }

  out_buf.append(is_exp_pipe.buffer());
  out_buf.append(is_optional_minus.buffer());
  out_buf.append(is_optional_plus.buffer());
  out_buf.append(is_1_or_more_digits_pipe.buffer());

  return true;
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
constexpr bool parse_number(ParserState<InputIt>& state, Buffer& out_buf) {
  EOF_CHECK(state);

  int sign = 1;
  u8 b = state.next();

  // Must be minus or digits.
  if (is_minus(b)) {
    sign = -1;
    out_buf.push_back(b);
  } else if (!is_digit(b)) {
    return false;
  }

  state.put(b);

  // Parse `int` part.
  if (!parse_number(state, out_buf)) {
    return false;
  }

  IF_EOF_RETURN(state, true);

  // Parse optional frac part.
  b = state.next();
  bool has_frac = false;

  if (is_decimal(b)) {
    state.put(b);

    if (!parse_frac(state, out_buf)) {
      return false;
    }

    has_frac = true;
  }

  // Parse optional exponent.
  IF_EOF_RETURN(state, true);

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