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
  explicit JsonValue(JsonNull v) : storage_{v} {}
  explicit JsonValue(bool v) : storage_{v} {}
  explicit JsonValue(JsonNumber v) : storage_{v} {}
  explicit JsonValue(std::string const& v) : storage_{v} {}
  explicit JsonValue(JsonArray* v) : storage_{v} {}
  explicit JsonValue(JsonObject* v) : storage_{v} {}

  JsonType type() const { return static_cast<JsonType>(storage_.index()); }

  JsonNull as_null() const {
    assert(type() == JsonType::kNull);

    return std::get<JsonNull>(storage_);
  }

  bool as_bool() const {
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

 private:
  using Storage = std::variant<JsonNull, bool, JsonNumber, std::string,
                               JsonArray*, JsonObject*>;
  Storage storage_;
};

}  // namespace json
