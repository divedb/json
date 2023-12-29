#pragma once

#include <string>

#include "json/types.h"

namespace json {

enum class ErrorType : u8 { kDefault, kUnknownByte, kOverflow, kUnderflow };

inline std::string error_type_to_string(ErrorType etype) {
  switch (etype) {
    case ErrorType::kDefault:
      return "No Error";
    case ErrorType::kUnknownByte:
      return "Unknown Byte";
    case ErrorType::kOverflow:
      return "Overflow";
    case ErrorType::kUnderflow:
      return "Underflow";
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

}  // namespace json