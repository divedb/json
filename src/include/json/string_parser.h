#pragma once

#include <iomanip>
#include <sstream>
#include <string>

#include "json/utf.h"
#include "json/util.h"

namespace json {

// TODO(gc): do we need static storage here?
inline constexpr auto is_quotation_mark = is_byte<'"'>;
inline constexpr auto is_escape = is_byte<'\\'>;
inline constexpr auto is_u = is_byte<'u'>;

template <typename InputIt>
inline constexpr auto is_escape_pipe =
    [](ParseState<InputIt> ps) { return has_one(ps, is_escape); };

template <typename InputIt>
inline constexpr auto is_u_pipe =
    [](ParseState<InputIt> ps) { return has_one(ps, is_u); };

template <typename InputIt>
inline constexpr auto is_4_hex_pipe =
    [](ParseState<InputIt> ps) { return has_fixed<4>(ps, is_hex); };

// Parse the unicode between first and last.
// Example:
// s = "1234"
// cp = unicode_to_codepoint(s, s + strlen(s))
// EXPECT_EQ(0x1234, cp)
template <typename InputIt>
inline i32 unicode_to_codepoint(InputIt first, InputIt last) {
  std::string tmp(first, last);
  std::istringstream iss(tmp);

  i32 n = 0;
  iss >> std::hex >> n;

  return n;
}

// Plane 0 is the Basic Multilingual Plane (BMP), which contains most commonly
// used characters.
// The unicode format is: \\udddd
//
// TODO(gc): if we parse surrogate first and there are 4 hex digits.
// Then these 4 hex digits are consumed in stream mode, `parse_bmp` function
// has no chance to get these 4 hex digits back.
template <typename InputIt>
inline constexpr void internal_parse_unicode(ParseState<InputIt>& ps,
                                             Buffer& out_buf) {
  if (ps = ps | is_4_hex_pipe<InputIt>; !ps.is_ok()) {
    return;
  }

  i32 r1 = unicode_to_codepoint(ps.ocursor, ps.ncursor);

  if (!is_surrogate(r1)) {
    out_buf.append(UTF8::encode(r1));
  } else if (is_high_surrogate(r1)) {
    if (ps = ps | is_escape_pipe<InputIt> | is_u_pipe<InputIt> |
             is_4_hex_pipe<InputIt>;
        ps.is_ok()) {
      i32 r2 = unicode_to_codepoint(ps.ocursor, ps.ncursor);

      if (is_low_surrogate(r2)) {
        out_buf.append(UTF8::encode(UTF16::decode(r1, r2)));
      } else {
        ps.status = Status::kError;
      }
    }
  } else {
    ps.status = Status::kError;
  }
}

template <typename InputIt>
inline constexpr void internal_parse_escape(ParseState<InputIt>& ps,
                                            Buffer& out_buf) {
  if (!ps.has_next()) {
    ps.status = Status::kEOF;
    return;
  }

  u8 b = ps.next();

  switch (b) {
    case '"':
      out_buf.push_back('\"');
      break;
    case '\\':
      out_buf.push_back('\\');
      break;
    case '/':
      out_buf.push_back('/');
      break;
    case 'b':
      out_buf.push_back('\b');
      break;
    case 'f':
      out_buf.push_back('\f');
      break;
    case 'n':
      out_buf.push_back('\n');
      break;
    case 'r':
      out_buf.push_back('\r');
      break;
    case 't':
      out_buf.push_back('\t');
      break;
    case 'u':
      internal_parse_unicode(ps, out_buf);
      break;
    default:
      ps.status = Status::kError;
      break;
  }
}

// string = quotation-mark *char quotation-mark
// char   =   unescaped
//          | escape (
//                      "       quotation mark      U+0022
//                      \       reverse solidus     U+005C
//                      /       solidus             U+002F
//                      b       backspace           U+0008
//                      f       form feed           U+000C
//                      n       line feed           U+000A
//                      r       carriage return     U+000D
//                      t       tab                 U+0009
//                      uXXXX                       U+XXXX
//                   )
// escape           = %x5C
// question-mark    = %x22
// unescaped        s= %x20-21 | %x23-5B | %x5D-10FFFF
template <typename InputIt>
inline constexpr bool internal_parse_string(ParseState<InputIt>& ps,
                                            Buffer& out_buf) {
  if (ps = has_one(ps, is_quotation_mark); !ps.is_ok()) {
    return false;
  }

  out_buf.push_back('"');

  while (ps.is_ok() && ps.has_next()) {
    u8 b = ps.next();

    if (is_escape(b)) {
      internal_parse_escape(ps, out_buf);
    } else if (is_quotation_mark(b)) {
      out_buf.push_back('"');

      return true;
    } else {
      out_buf.push_back(b);
    }
  }

  if (ps.is_ok()) {
    ps.status = Status::kEOF;
  }

  return false;
}

}  // namespace json
