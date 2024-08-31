#pragma once

#include <string_view>
#include <vector>

#include "json/node/json_value.hpp"

namespace json {

class JsonNode {
 public:
  JsonNode() = default;
  explicit JsonNode(JsonValue const& json_value) : json_value_(json_value) {}

  constexpr bool is_object() const { return json_value_.is_object(); }
  constexpr bool is_array() const { return json_value_.is_array(); }
  constexpr bool is_string() const { return json_value_.is_string(); }
  constexpr bool is_number() const { return json_value_.is_number(); }
  constexpr bool is_bool() const { return json_value_.is_bool(); }
  constexpr bool is_null() const { return json_value_.is_null(); }

  std::string get_string() const;
  JsonNumber get_number() const;
  bool get_bool() const;
  JsonArray get_array() const;
  JsonObject get_object() const;

  void clone();
  void remove();
  void insert_before();
  void insert_after();

 private:
  JsonValue json_value_;
  JsonNode* parent_;
  JsonNode* sibling_;
  std::vector<JsonNode*> children_;
};

}  // namespace json