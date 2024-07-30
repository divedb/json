#pragma once

#include <cstdlib>  // strtod
#include <vector>

#include "json/common/constants.hpp"
#include "json/common/number_converter.hpp"
#include "json/nodes/json_value.hpp"
#include "json/parser/parse_error.hpp"

namespace json {

/// int           = zero | ( digit1-9 *DIGIT )
/// zero          = %x30                                 ; 0
/// digit1-9      = %x31-39                              ; 1-9
template <typename InputIt>
ErrorCode parse_int(InputIt& first, InputIt last, std::vector<char>& buf) {
  if (first == last) {
    return ErrorCode::kEOF;
  }

  char ch = *first++;

  if (!std::isdigit(ch)) {
    return ErrorCode::kInvalid;
  }

  buf.push_back(ch);

  if (ch == '0') {
    // Leading zeros are not allowed.
    if (first != last && std::isdigit(*first)) {
      return ErrorCode::kInvalid;
    }
  } else {
    while (first != last && std::isdigit(*first)) {
      buf.push_back(*first++);
    }
  }

  return ErrorCode::kOk;
}

/// frac          = decimal-point 1*DIGIT
/// decimal-point = %x2E                                 ;  .
template <typename InputIt>
ErrorCode parse_optional_frac(InputIt& first, InputIt last,
                              std::vector<char>& buf) {
  if (first != last && *first == kPeriod) {
    buf.push_back(kPeriod);
    auto ofirst = ++first;

    while (first != last && std::isdigit(*first)) {
      buf.push_back(*first++);
    }

    // At least 1 digit is followed.
    if (ofirst == first) {
      return ErrorCode::kInvalid;
    }
  }

  // Either `first == last or *first != '.'`
  return ErrorCode::kOk;
}

/// e             = %x65 / %x45                          ; e E
/// exp           = e [ minus | plus ] 1*DIGIT
/// minus         = %x2D                                 ; -
/// plus          = %x2B                                 ; +
template <typename InputIt>
ErrorCode parse_optional_exponent(InputIt& first, InputIt last,
                                  std::vector<char>& buf) {
  if (first != last && (*first == 'e' || *first == 'E')) {
    buf.push_back(*first);
    auto ofirst = ++first;

    while (first != last && std::isdigit(*first)) {
      buf.push_back(*first++);
    }

    // At least 1 digit is followed.
    if (ofirst == first) {
      return ErrorCode::kInvalid;
    }
  }

  return ErrorCode::kOk;
}

/// number        = [ minus ] int [ frac ] [ exp ]
/// minus         = %x2D                                 ; -
template <typename InputIt>
InputIt parse_number(InputIt first, InputIt last, JsonValue& json_value,
                     ErrorCode& err) {
  if (first == last) {
    err = ErrorCode::kEOF;

    return first;
  }

  char ch = *first;
  std::vector<char> buf;

  if (ch == '-') {
    ++first;
    buf.push_back(ch);
  }

  if (err = parse_int(first, last, buf); err != ErrorCode::kOk) {
    return first;
  }

  if (err = parse_optional_frac(first, last, buf); err != ErrorCode::kOk) {
    return first;
  }

  if (err = parse_optional_exponent(first, last, buf); err != ErrorCode::kOk) {
    return first;
  }

  // Terminate the string.
  NumberConverter conv;
  buf.push_back(0);
  char const* cstr = buf.data();

  if (is_float(cstr)) {
    auto v = conv.operator()<double>(cstr);
    json_value = JsonValue{v};
  } else {
    auto v = conv.operator()<int64_t>(cstr);
    json_value = JsonValue{v};
  }

  if (conv.is_overflow()) {
    err = ErrorCode::kOverflow;
  }

  if (conv.is_underflow()) {
    err = ErrorCode::kUnderflow;
  }

  return first;
}

}  // namespace json