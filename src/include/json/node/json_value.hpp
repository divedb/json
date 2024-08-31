#pragma once

#include <cassert>
#include <iostream>
#include <string>
#include <type_traits>
#include <variant>

#include "json/node/json_null.hpp"
#include "json/node/json_number.hpp"

namespace json {

class JsonArray;
class JsonObject;

enum class JsonType : int { kNull, kBool, kNumber, kString, kArray, kObject };

class JsonValue {
 public:
  constexpr JsonValue() : storage_{Dummy{}} {}
  constexpr explicit JsonValue(JsonNull v) : storage_{v} {}
  constexpr explicit JsonValue(bool v) : storage_{v} {}

  template <typename T>
  requires std::is_integral_v<T> || std::is_floating_point_v<T>
  constexpr explicit JsonValue(T v) : storage_{JsonNumber{v}} {}

  constexpr explicit JsonValue(JsonNumber const& v) : storage_{v} {}
  constexpr explicit JsonValue(std::string const& v) : storage_{v} {}
  explicit JsonValue(char const* v) : storage_{std::string(v)} {}
  explicit JsonValue(std::string_view v) : storage_{std::string(v)} {}
  constexpr explicit JsonValue(JsonArray* v) : storage_{v} {}
  constexpr explicit JsonValue(JsonObject* v) : storage_{v} {}

  /// Check if current node is type of object.
  /// \return `true` if this node is an object; otherwise, return `false`.
  constexpr bool is_object() const { return type() == JsonType::kObject; }

  /// Check if current node is type of array.
  /// \return `true` if this node is an array; otherwise, return `false`.
  constexpr bool is_array() const { return type() == JsonType::kArray; }

  /// Check if current node is type of string.
  /// \return `true` if this node is a string; otherwise, return `false`.
  constexpr bool is_string() const { return type() == JsonType::kString; }

  /// Check if current node is type of number.
  /// \return `true` if this node is a number; otherwise, return `false`.
  constexpr bool is_number() const { return type() == JsonType::kNumber; }

  /// Check if current node is type of bool.
  /// \return `true` if this node is literal "true"; otherwise, return `false`.
  constexpr bool is_bool() const { return type() == JsonType::kBool; }

  /// Check if current node is type of null.
  /// \return `true` if this node is literal "null"; otherwise, return `false`.
  constexpr bool is_null() const { return type() == JsonType::kNull; }

  JsonValue& operator=(JsonNull v) {
    storage_ = v;

    return *this;
  }

  JsonValue& operator=(bool v) {
    storage_ = v;

    return *this;
  }

  JsonValue& operator=(int v) {
    storage_ = JsonNumber{v};

    return *this;
  }

  JsonValue& operator=(double v) {
    storage_ = JsonNumber{v};

    return *this;
  }

  JsonType type() const { return static_cast<JsonType>(storage_.index()); }

  JsonNull& as_null() {
    assert(type() == JsonType::kNull);

    return std::get<JsonNull>(storage_);
  }

  JsonNull const& as_null() const {
    assert(type() == JsonType::kNull);

    return std::get<JsonNull>(storage_);
  }

  bool& as_bool() {
    assert(type() == JsonType::kBool);

    return std::get<bool>(storage_);
  }

  bool const& as_bool() const {
    assert(type() == JsonType::kBool);

    return std::get<bool>(storage_);
  }

  JsonNumber& as_number() {
    assert(type() == JsonType::kNumber);

    return std::get<JsonNumber>(storage_);
  }

  JsonNumber const& as_number() const {
    assert(type() == JsonType::kNumber);

    return std::get<JsonNumber>(storage_);
  }

  std::string& as_string() {
    assert(type() == JsonType::kString);

    return std::get<std::string>(storage_);
  }

  std::string const& as_string() const {
    assert(type() == JsonType::kString);

    return std::get<std::string>(storage_);
  }

  JsonArray*& as_array() {
    assert(type() == JsonType::kArray);

    return std::get<JsonArray*>(storage_);
  }

  JsonArray const* as_array() const {
    assert(type() == JsonType::kArray);

    return std::get<JsonArray*>(storage_);
  }

  JsonObject*& as_object() {
    assert(type() == JsonType::kObject);

    return std::get<JsonObject*>(storage_);
  }

  JsonObject const* as_object() const {
    assert(type() == JsonType::kObject);

    return std::get<JsonObject*>(storage_);
  }

  constexpr bool is_simple_type() const {
    return is_null() || is_bool() || is_number() || is_string();
  }

  constexpr bool is_aggregate_type() const { return !is_simple_type(); }

  friend std::ostream& operator<<(std::ostream& os,
                                  JsonValue const& json_value) {
    std::visit([&os](auto&& arg) { os << arg; }, json_value.storage_);

    return os;
  }

  friend constexpr bool operator==(JsonValue const& lhs, JsonValue const& rhs);
  friend constexpr bool operator!=(JsonValue const& lhs, JsonValue const& rhs);

 private:
  struct Dummy {
    constexpr std::strong_ordering operator<=>(Dummy const&) const = default;

    friend std::ostream& operator<<(std::ostream& os, Dummy const&) {
      return os << "dummy";
    }
  };

  using Storage = std::variant<JsonNull, bool, JsonNumber, std::string,
                               JsonArray*, JsonObject*, Dummy>;
  Storage storage_;
};

}  // namespace json
