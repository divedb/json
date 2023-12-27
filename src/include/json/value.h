#pragma once

#include <iostream>
#include <string>
#include <type_traits>
#include <variant>

namespace json {

enum JsonType : uint8_t {
  kInvalid,
  kString,
  kNumber,
  kBool,
  kNull,
  kObject,
  kArray
};

using BigInteger = long long;
using String = std::string;

struct Number {
 public:
  Number() = default;
  explicit Number(BigInteger value) : is_integer_{true}, value_{value} {}
  explicit Number(double value) : is_integer_{false}, value_{value} {}

  bool is_integer() const { return is_integer_; }

  template <typename T>
  T get() const {
    if constexpr (std::is_same_v<T, BigInteger>) {
      return std::get<BigInteger>(value_);
    }

    if constexpr (std::is_same_v<T, double>) {
      return std::get<double>(value_);
    }
  }

  friend bool operator==(const Number& lhs, const Number& rhs) {
    return lhs.is_integer_ == rhs.is_integer_ && lhs.value_ == rhs.value_;
  }

  friend bool operator!=(const Number& lhs, const Number& rhs) {
    return !(lhs == rhs);
  }

 private:
  bool is_integer_;
  std::variant<BigInteger, double> value_;
};

class JsonValue {
 public:
  JsonValue() = default;

  explicit JsonValue(Number num) : type_{kNumber}, value_{num} {}

  bool is_valid() const { return type_ != JsonType::kInvalid; }
  JsonType type() const { return type_; }
  void set_type(JsonType type) { type_ = type; }

  template <typename T>
  T get() const {
    if constexpr (std::is_same_v<T, Number>) {
      return std::get<Number>(value_);
    }

    if constexpr (std::is_same_v<T, String>) {
      return std::get<String>(value_);
    }
  }

  void print() const {
    switch (type_) {
      case kInvalid:
        std::cout << "invalid\n";
        break;

      case kString:
        std::cout << "string\n";
        break;

      case kNumber:
        std::cout << "number\n";
        break;

      case kBool:
        std::cout << "bool\n";
        break;

      case kNull:
        std::cout << "null\n";
        break;

      default:
        std::cout << "default\n";
        break;
    }
  }

 private:
  JsonType type_{JsonType::kInvalid};
  std::variant<String, Number> value_;
};

}  // namespace json