#pragma once

namespace json {

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