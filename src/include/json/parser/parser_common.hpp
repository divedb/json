#pragma once

#include "json/common/constants.hpp"
#include "json/parser/array_parser.hpp"
#include "json/parser/number_parser.hpp"
#include "json/parser/string_parser.hpp"

namespace json {

#define CHECK_EOF(first, last, err) \
  do {                              \
    if (first == last) {            \
      err = ErrorCode::kEOF;        \
      return first;                 \
    }                               \
  } while (0)

enum class Stage : int {
  kParseNull,
  kParseBool,
  kParseNumber,
  kParseString,
  kParseArray,
  kParseObject
};

/// @brief kInvalid: invalid number
enum class ErrorCode : int { kOk, kEOF, kInvalid, kUnderflow, kOverflow };

class ParseError {
 public:
  ParseError() = default;
  ParseError(Stage stage, ErrorCode ecode) : stage_{stage}, ecode_{ecode} {}

  Stage stage() const { return stage_; }
  ErrorCode error_code() const { return ecode_; }

 private:
  Stage stage_;
  ErrorCode ecode_;
};

}  // namespace json