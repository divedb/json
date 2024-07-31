#pragma once

#include <algorithm>
#include <cassert>

#include "json/common//util.hpp"
#include "json/common/constants.hpp"
#include "json/nodes/json_value.hpp"
#include "json/parser/parse_error.hpp"
#include "json/unicode/utf8.hpp"

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
  const int ndigits = 4;
  auto dist = std::distance(first, last);

  if (dist < ndigits) {
    return ErrorCode::kEOF;
  }

  if (std::any_of(first, first + ndigits,
                  [](char ch) { return !std::isxdigit(ch); })) {
    return ErrorCode::kInvalid;
  }

  int codepoint = 0;
  const int base = 16;

  for (int i = 0; i < ndigits; i++, first++) {
    codepoint = codepoint * base + hex_char_to_int(*first);
  }

  if (!UTF8::is_valid_rune(codepoint)) {
    return ErrorCode::kInvalid;
  }

  char unicode[ndigits];
  int n = UTF8::encode(std::begin(unicode), codepoint);
  buf.insert(buf.end(), std::begin(unicode), std::begin(unicode) + n);

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
  char ch = *first++;

  assert(ch == kQuote);

  CHECK_EOF(first, last, err);

  std::string buf;
  err = ErrorCode::kOk;

  while (first != last && err == ErrorCode::kOk) {
    ch = *first++;

    if (ch == '\\') {
      CHECK_EOF(first, last, err);

      // Read next character.
      ch = *first++;

      if (ch == kQuote) {
        buf += kQuote;
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
        err = parse_unicode(first, last, buf);
      } else {
        err = ErrorCode::kInvalid;
      }
    } else if (ch == kQuote) {
      json_value = JsonValue{buf};

      return first;
    } else {
      buf.push_back(ch);
    }
  }

  CHECK_EOF(first, last, err);

  return first;
}

}  // namespace json
