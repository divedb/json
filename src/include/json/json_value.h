#pragma once

#include <iostream>
#include <string>
#include <type_traits>
#include <variant>

#include "json/types.h"

namespace json {

class JsonValue;

using String = Buffer;
using BigInteger = long long;
using LongDouble = long double;

enum class JsonType : u8 {
  kInvalid,
  kString,
  kNumber,
  kBool,
  kNull,
  kObject,
  kArray
};

struct Number {
 public:
  Number() = default;
  explicit Number(BigInteger value) : value_{value} {}
  explicit Number(LongDouble value) : value_{value} {}

  bool is_integer() const { return value_.index() == 0; }

  constexpr bool operator==(const Number& other) const {
    return value_ == other.value_;
  }

  constexpr bool operator!=(const Number& other) const {
    return !(*this == other);
  }

  template <typename T>
  T get() const {
    if constexpr (std::is_same_v<T, BigInteger>) {
      return std::get<BigInteger>(value_);
    }

    if constexpr (std::is_same_v<T, LongDouble>) {
      return std::get<LongDouble>(value_);
    }
  }

  constexpr void print() const {
    if (value_.index() == 0) {
      std::cout << get<BigInteger>() << std::endl;
    } else {
      std::cout << get<LongDouble>() << std::endl;
    }
  }

 private:
  std::variant<BigInteger, LongDouble> value_;
};

class JsonValue {
 public:
  JsonValue() = default;

  JsonValue(Number num) : type_{JsonType::kNumber}, value_{num} {}
  JsonValue(const String& str) : type_{JsonType::kString}, value_{str} {}

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
      case JsonType::kInvalid:
        std::cout << "invalid\n";
        break;

      case JsonType::kString:
        std::cout << "string\n";
        break;

      case JsonType::kNumber:
        std::cout << "number\n";
        std::get<1>(value_).print();
        break;

      case JsonType::kBool:
        std::cout << "bool\n";
        break;

      case JsonType::kNull:
        std::cout << "null\n";
        break;

      default:
        std::cout << "default\n";
        break;
    }
  }

  constexpr bool operator==(const JsonValue& other) const noexcept {
    return type_ == other.type_ && value_ == other.value_;
  }

  constexpr bool operator!=(const JsonValue& other) const noexcept {
    return !(*this == other);
  }

 private:
  JsonType type_{JsonType::kInvalid};
  std::variant<String, Number> value_;
};

}  // namespace json
