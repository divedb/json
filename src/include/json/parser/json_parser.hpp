#pragma once

#include <type_traits>

#include "json/nodes/json_value.hpp"
#include "json/parser/parser_common.hpp"

namespace json {

struct Location {
  void update(int ch) {
    if (std::isspace(ch)) {
      y_pos++;
      x_pos = 0;
    } else {
      x_pos++;
    }

    source_location++;
  }

  int x_pos{};
  int y_pos{};
  int source_location{};
};

// class Parser {
//  public:
//   explicit Parser(MemoryContext& mem_ctx) : mem_ctx_{mem_ctx} {}

//   JsonNode* parse(CharStream& stream);

//  private:
//   void parse_null(CharStream& stream, ParserState& state, JsonNode*& jnode);
//   void parse_bool(CharStream& stream, ParserState& state, JsonNode*& jnode);
//   void parse_number(CharStream& stream, ParserState& state, JsonNode*&
//   jnode);

//   JsonNode* parse_string(CharStream& stream, ParserState& state);
//   JsonNode* parse_array(CharStream& stream, ParserState& state);
//   JsonNode* parse_object(CharStream& stream, ParserState& state);

//   MemoryContext& mem_ctx_;
// };

template <typename Allocator>
class JsonParser {
 public:
  explicit JsonParser(Allocator const& alloc) : alloc_{alloc} {}

  template <typename InputIt>
  JsonValue parse(InputIt first, InputIt last, ErrorCode& err) {}

 private:
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

    if (err = parse_optional_exponent(first, last, buf);
        err != ErrorCode::kOk) {
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

  template <typename InputIt>
  ErrorCode parse_null(InputIt& first, InputIt last, JsonValue& json_value) {
    auto dist = std::distance(first, last);
    auto size = strlen(kNullValue);

    if (dist < size) {
      return ErrorCode::kEOF;
    }

    if (!std::equal(kNullValue, kNullValue + size, first)) {
      return ErrorCode::kInvalid;
    }

    json_value = JsonValue::null();

    return ErrorCode::kOk;
  }

  template <typename InputIt>
  ErrorCode parse_bool(InputIt& first, InputIt last, JsonValue& json_value) {
    char ch = *first++;
    char const* expect = kFalseValue;
    bool is_true = ch == 't';

    if (is_true) {
      expect = kTrueValue;
    }

    auto dist = std::distance(first, last);
    auto size = strlen(expect);

    if (dist < size) {
      return ErrorCode::kEOF;
    }

    if (!std::equal(expect, expect + size, first)) {
      return ErrorCode::kInvalid;
    }

    json_value.as_bool() = is_true;

    return ErrorCode::kOk;
  }

  template <typename InputIt>
  InputIt parse_common(InputIt& first, InputIt last, JsonValue& json_value,
                       ErrorCode& err) {
    CHECK_EOF(first, last, err);

    char ch = *first++;

    if (ch == kQuote) {
      return parse_string(first, last, json_value, err);
    } else if (std::isdigit(ch) || ch == '-') {
      return parse_number(first, last, json_value, err);
    } else if (ch == kOpenBracket) {
      return parse_array(first, last, json_value, err, alloc);
    }
  }

  Allocator alloc_;
};

}  // namespace json