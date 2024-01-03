#pragma once

#include <string>

#include "json/types.h"

namespace json {

enum class ErrorType : u8 {
  kParseNull,
  kParseBool,
  kParseNumber,
  kParseString,
  kParseArray,
  kParseObject
};

inline std::string error_type_to_string(ErrorType etype) {
  switch (etype) {
    case ErrorType::kParseNull:
      return "[Parse NULL]";
    case ErrorType::kParseBool:
      return "[Parse BOOL]";
    case ErrorType::kParseNumber:
      return "[Parse NUMBER]";
    case ErrorType::kParseString:
      return "[Parse STRING]";
    case ErrorType::kParseArray:
      return "[Parse ARRAY]";
    case ErrorType::kParseObject:
      return "[Parse OBJECT]";
    default:
      return "";
  }
}

class Error {
 public:
  Error() = default;
  Error(ErrorType etype, const std::string& emsg)
      : etype_(etype), emsg_(emsg) {}

  std::string to_string() const {
    return error_type_to_string(etype_) + ": " + emsg_;
  }

 private:
  ErrorType etype_{};
  std::string emsg_;
};

class NumberError : public Error {
 public:
  NumberError(const std::string& emsg) : Error(ErrorType::kParseNumber, emsg) {}
};

}  // namespace json