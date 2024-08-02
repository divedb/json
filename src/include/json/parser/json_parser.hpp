#pragma once

#include <type_traits>

#include "json/common/constants.hpp"
#include "json/common/number_converter.hpp"
#include "json/nodes/json_array.hpp"
#include "json/nodes/json_object.hpp"
#include "json/parser/parser_common.hpp"
#include "json/unicode/utf8.hpp"

namespace json {

class JsonParser {
 public:
  template <typename InputIt, typename Allocator>
  static std::pair<JsonValue, ErrorCode> parse(InputIt first, InputIt last,
                                               Allocator& alloc) {
    JsonValue json_value;
    ErrorCode err = parse_common(first, last, alloc, json_value);

    return std::make_pair(json_value, err);
  }

 private:
  template <typename InputIt>
  static ErrorCode parse_null(InputIt& first, InputIt last,
                              JsonValue& json_value) {
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
  static ErrorCode parse_bool(InputIt& first, InputIt last,
                              JsonValue& json_value) {
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
  static ErrorCode parse_unicode(InputIt& first, InputIt last,
                                 std::string& buf) {
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
  static ErrorCode parse_string(InputIt& first, InputIt last,
                                JsonValue& json_value) {
    // A string begins and ends with quotation marks. All Unicode characters may
    // be placed within the quotation marks, except for the characters that must
    // be escaped: quotation mark, reverse solidus, and the control characters
    // (U+0000 through U+001F).
    char ch = *first++;

    assert(ch == kQuote);

    CHECK_EOF(first, last);

    std::string buf;

    while (first != last) {
      ch = *first++;

      if (ch == '\\') {
        CHECK_EOF(first, last);

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
          if (ErrorCode err = parse_unicode(first, last, buf);
              err != ErrorCode::kOk) {
            return err;
          }
        } else {
          return ErrorCode::kInvalid;
        }
      } else if (ch == kQuote) {
        json_value.as_string() = std::move(buf);

        return ErrorCode::kOk;
      } else {
        buf.push_back(ch);
      }
    }

    return ErrorCode::kEOF;
  }

  /// int           = zero | ( digit1-9 *DIGIT )
  /// zero          = %x30                                 ; 0
  /// digit1-9      = %x31-39                              ; 1-9
  template <typename InputIt>
  static ErrorCode parse_int(InputIt& first, InputIt last, std::string& buf) {
    CHECK_EOF(first, last);

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
  static ErrorCode parse_optional_frac(InputIt& first, InputIt last,
                                       std::string& buf) {
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
  static ErrorCode parse_optional_exponent(InputIt& first, InputIt last,
                                           std::string& buf) {
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
  static ErrorCode parse_number(InputIt& first, InputIt last,
                                JsonValue& json_value) {
    char ch = *first;
    std::string buf;

    if (ch == '-') {
      ++first;
      buf.push_back(ch);
    }

    if (ErrorCode err = parse_int(first, last, buf); err != ErrorCode::kOk) {
      return err;
    }

    if (ErrorCode err = parse_optional_frac(first, last, buf);
        err != ErrorCode::kOk) {
      return err;
    }

    if (ErrorCode err = parse_optional_exponent(first, last, buf);
        err != ErrorCode::kOk) {
      return err;
    }

    // Terminate the string.
    NumberConverter conv;
    char const* cstr = buf.c_str();

    if (is_float(cstr)) {
      auto v = conv.operator()<double>(cstr);
      json_value.as_number() = v;
    } else {
      auto v = conv.operator()<int64_t>(cstr);
      json_value.as_number() = v;
    }

    if (conv.is_overflow()) {
      return ErrorCode::kOverflow;
    }

    if (conv.is_underflow()) {
      return ErrorCode::kUnderflow;
    }

    return ErrorCode::kOk;
  }

  /// An array structure is represented as square brackets surrounding zero or
  /// more values (or elements). Elements are separated by commas.
  /// array = begin-array [ value *( value-separator value ) ] end-array
  /// There is no requirement that the values in an array be of the same type.
  template <typename InputIt, typename Allocator>
  static ErrorCode parse_array(InputIt& first, InputIt last, Allocator& alloc,
                               JsonValue& json_value) {
    char ch = *first++;
    // Read [.
    assert(ch == kOpenBracket);
    first = skip_space(first, last);
    CHECK_EOF(first, last);
    // Read next character.
    ch = *first;

    if (ch == kCloseBracket) {
      // Advance to next character.
      first++;

      return ErrorCode::kOk;
    } else {
      while (first != last) {
        JsonValue element;
        ErrorCode err = parse_common(first, last, alloc, element);

        if (err != ErrorCode::kOk) {
          return err;
        }

        json_value.as_array()->append(element);
        // Space is allowed between array element and comma.
        first = skip_space(first, last);
        // Exclude case like: [1,
        CHECK_EOF(first, last);

        ch = *first++;

        if (ch == kCloseBracket) {
          return ErrorCode::kOk;
        }

        if (ch != kListSeparator) {
          return ErrorCode::kInvalid;
        }

        first = skip_space(first, last);
      }

      return ErrorCode::kEOF;
    }
  }

  template <typename InputIt, typename Allocator>
  static ErrorCode parse_object(InputIt& first, InputIt last, Allocator& alloc,
                                JsonValue& json_value) {
    char ch = *first++;
    // Read {.
    assert(ch == kOpenBrace);
    first = skip_space(first, last);
    CHECK_EOF(first, last);
    // Read next character.
    ch = *first;

    if (ch == kCloseBrace) {
      // Advance to next character.
      first++;

      return ErrorCode::kOk;
    } else {
      if (ch != kQuote) {
        return ErrorCode::kInvalid;
      }

      while (first != last) {
        JsonValue key;
        ErrorCode err = parse_string(first, last, key);

        if (err != ErrorCode::kOk) {
          return err;
        }

        first = skip_space(first, last);
        CHECK_EOF(first, last);
        ch = *first++;

        if (ch != kKeyValueSeparator) {
          return ErrorCode::kInvalid;
        }

        first = skip_space(first, last);

        JsonValue value;
        err = parse_common(first, last, alloc, value);

        if (err != ErrorCode::kOk) {
          return err;
        }

        json_value.as_object()->append(key.as_string(), value);

        first = skip_space(first, last);
        CHECK_EOF(first, last);
        ch = *first++;

        if (ch == kCloseBrace) {
          return ErrorCode::kOk;
        }

        if (ch != kListSeparator) {
          return ErrorCode::kInvalid;
        }

        first = skip_space(first, last);
      }

      return ErrorCode::kEOF;
    }
  }

  template <typename InputIt, typename Allocator>
  static ErrorCode parse_common(InputIt& first, InputIt last, Allocator& alloc,
                                JsonValue& json_value) {
    CHECK_EOF(first, last);

    char ch = *first;

    if (ch == 'n') {
      json_value = JsonValue::get_default<JsonNull>();

      return parse_null(first, last, json_value);
    } else if (ch == 't' || ch == 'f') {
      json_value = JsonValue::get_default<bool>();

      return parse_bool(first, last, json_value);
    } else if (std::isdigit(ch) || ch == '-') {
      json_value = JsonValue::get_default<JsonNumber>();

      return parse_number(first, last, json_value);
    } else if (ch == kQuote) {
      json_value = JsonValue::get_default<std::string>();

      return parse_string(first, last, json_value);
    } else if (ch == kOpenBracket) {
      json_value = JsonValue::get_default<JsonArray>();
      void* ptr = alloc.malloc(sizeof(JsonArray));
      json_value.as_array() = new (ptr) JsonArray();

      return parse_array(first, last, alloc, json_value);
    } else if (ch == kOpenBrace) {
      json_value = JsonValue::get_default<JsonObject>();

      void* ptr = alloc.malloc(sizeof(JsonObject));
      json_value.as_object() = new (ptr) JsonObject();

      return parse_object(first, last, alloc, json_value);
    }

    return ErrorCode::kInvalid;
  }

  template <typename InputIt>
  static InputIt skip_space(InputIt first, InputIt last) {
    return std::find_if_not(first, last,
                            [](char ch) { return std::isdigit(ch); });
  }
};

}  // namespace json