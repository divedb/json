#pragma once

#include <cassert>
#include <iostream>
#include <string>
#include <type_traits>
#include <variant>

#include "json/nodes/json_null.hpp"
#include "json/nodes/json_number.hpp"

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
  constexpr explicit JsonValue(JsonArray* v) : storage_{v} {}
  constexpr explicit JsonValue(JsonObject* v) : storage_{v} {}

  JsonType type() const { return static_cast<JsonType>(storage_.index()); }

  JsonNull as_null() const {
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

  JsonNumber as_number() const {
    assert(type() == JsonType::kNumber);

    return std::get<JsonNumber>(storage_);
  }

  std::string as_string() const {
    assert(type() == JsonType::kString);

    return std::get<std::string>(storage_);
  }

  JsonArray* as_array() const {
    assert(type() == JsonType::kArray);

    return std::get<JsonArray*>(storage_);
  }

  JsonObject* as_object() const {
    assert(type() == JsonType::kObject);

    return std::get<JsonObject*>(storage_);
  }

  constexpr bool is_object() const { return type() == JsonType::kObject; }
  constexpr bool is_array() const { return type() == JsonType::kArray; }
  constexpr bool is_string() const { return type() == JsonType::kString; }
  constexpr bool is_number() const { return type() == JsonType::kNumber; }
  constexpr bool is_bool() const { return type() == JsonType::kBool; }
  constexpr bool is_null() const { return type() == JsonType::kNull; }

  static JsonValue null() {
    static JsonValue json_value{JsonNull{}};

    return json_value;
  }

  friend std::ostream& operator<<(std::ostream& os,
                                  JsonValue const& json_value) {
    std::visit([&os](auto&& arg) { os << arg; }, json_value.storage_);

    return os;
  }

  friend constexpr bool operator==(JsonValue const& lhs, JsonValue const& rhs) {
    return lhs.storage_ == rhs.storage_;
  }

  friend constexpr bool operator!=(JsonValue const& lhs, JsonValue const& rhs) {
    return !(lhs.storage_ == rhs.storage_);
  }

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
