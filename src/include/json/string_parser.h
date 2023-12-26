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
template <typename InputIt>
inline constexpr ParseState<InputIt> parse_bmp(ParseState<InputIt> ps,
                                               i32& out) {
  CHECK_PARSE_STATE(ps);

  if (ps = ps | is_4_hex_pipe<InputIt>; ps.is_ok()) {
    out = unicode_to_codepoint(ps.ocursor, ps.ncursor);
  }

  return ps;
}

template <typename InputIt, typename Predicate>
inline constexpr ParseState<InputIt> parse_surrogate_aux(ParseState<InputIt> ps,
                                                         Predicate&& p,
                                                         i32& out) {
  CHECK_PARSE_STATE(ps);

  if (ps = ps | is_4_hex_pipe<InputIt>; ps.is_ok()) {
    out = unicode_to_codepoint(ps.ocursor, ps.ncursor);

    if (!std::forward<Predicate>(p)(out)) {
      ps.status = Status::kError;
    }
  }

  return ps;
}

template <typename InputIt>
inline constexpr ParseState<InputIt> parse_surrogate(ParseState<InputIt> ps,
                                                     i32& out) {
  i32 r1;
  i32 r2;

  auto hp = [&r1](ParseState<InputIt> p) {
    return parse_surrogate_aux(p, is_high_surrogate, r1);
  };

  auto lp = [&r2](ParseState<InputIt> p) {
    return parse_surrogate_aux(p, is_low_surrogate, r2);
  };

  if (ps = ps | hp | is_escape_pipe<InputIt> | is_u_pipe<InputIt> | lp;
      ps.is_ok()) {
    out = UTF16::decode(r1, r2);
  }

  return ps;
}

template <typename InputIt>
inline constexpr ParseState<InputIt> parse_unicode(ParseState<InputIt> ps,
                                                   Buffer& out_buf) {
  i32 r;

  if (auto ps2 = parse_surrogate(ps, r); ps2.is_ok()) {
    out_buf.append(UTF8::encode(r));
    return ps2;
  }

  if (auto ps1 = parse_bmp(ps, r); ps1.is_ok()) {
    out_buf.append(UTF8::encode(r));

    return ps1;
  }

  return ps;
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
inline constexpr ParseState<InputIt> internal_parse_string(
    ParseState<InputIt> ps, Buffer& out_buf) {
  using VT = std::make_unsigned_t<typename ParseState<InputIt>::value_type>;

  if (ps = has_one(ps, is_quotation_mark); !ps.is_ok()) {
    return ps;
  }

  out_buf.push_back('"');

  while (ps.is_ok() && ps.has_next()) {
    VT b = ps.next();

    if (is_escape(b)) {
      if (!ps.has_next()) {
        ps.status = Status::kEOF;

        break;
      }

      b = ps.next();

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
          ps = parse_unicode(ps, out_buf);
          break;
        default:
          ps.status = Status::kError;
          break;
      }
    } else {
      // TODO(gc): rethink
      out_buf.push_back(b);

      if (is_quotation_mark(b)) {
        return ps;
      }
    }
  }

  // Reach here means, the parse state is either not ok or next character is not
  // available.
  if (!ps.has_next()) {
    ps.status = Status::kEOF;
  }

  return ps;
}

}  // namespace json
