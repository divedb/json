#pragma once

#include <algorithm>
#include <cassert>

#include "json/common/constants.hpp"
#include "json/nodes/json_value.hpp"
#include "json/parser/parse_error.hpp"

namespace json {

#define CHECK_EOF(first, last, err) \
  do {                              \
    if (first == last) {            \
      err = ErrorCode::kEOF;        \
      return first;                 \
    }                               \
  } while (0)

template <typename InputIt>
ErrorCode parse_unicode(InputIt& first, InputIt last, std::string& buf) {
  const int digits = 4;
  auto dist = std::distance(first, last);

  if (dist < digits || std::all_of(first, first + digits, std::isdigit)) {
    return ErrorCode::kInvalid;
  }

  int codepoint = 0;
  const int base = 10;

  for (int i = 0; i < digits; i++, first++) {
    codepoint = codepoint * base + *first;
  }

  return ErrorCode::kOk;
}

/// string = quotation-mark *char quotation-mark
/// char   =   unescaped
///          | escape (
///                      "       quotation mark      U+0022
///                      \       reverse solidus     U+005C
///                      /       solidus             U+002F
///                      b       backspace           U+0008
///                      f       form feed           U+000C
///                      n       line feed           U+000A
///                      r       carriage return     U+000D
///                      t       tab                 U+0009
///                      uXXXX                       U+XXXX
///                   )
/// escape           = %x5C
/// question-mark    = %x22
/// unescaped        = %x20-21 | %x23-5B | %x5D-10FFFF
template <typename InputIt>
InputIt parse_string(InputIt first, InputIt last, JsonValue& json_value,
                     ErrorCode& err) {
  // TODO(gc): needs EOF check?

  // A string begins and ends with quotation marks. All Unicode characters may
  // be placed within the quotation marks, except for the characters that must
  // be escaped: quotation mark, reverse solidus, and the control characters
  // (U+0000 through U+001F).
  assert(*first++ == kQuote);

  CHECK_EOF(first, last, err);

  char ch{};
  std::string buf;

  while (first != last) {
    ch = *first;

    if (ch == '\\') {
      ++first;
      CHECK_EOF(first, last, err);
      // Check next character.
      ch = *first;

      if (ch == '"') {
        buf += '"';
      } else if (ch == '\\') {
        buf += '\\';
      } else if (ch == '/') {
        buf += '/';
      } else if (ch == 'b') {
        buf += '\b';
      } else if (ch == 'f') {
        buf += '\f';
      } else if (ch == 'n') {
        buf += '\n';
      } else if (ch == 'r') {
        buf += '\r';
      } else if (ch == 't') {
        buf += '\t';
      } else if (ch == 'u') {
      } else {
        err = ErrorCode::kInvalid;

        return first;
      }
    } else {
    }
  }

  return first;
}

}  // namespace json
