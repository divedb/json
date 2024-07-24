#pragma once

#include <string_view>
#include <vector>

#include "json/api.hpp"

namespace json {

class JsonNode {
 public:
  JsonNode& parse(string_view json_data);

  constexpr bool is_object() const;
  constexpr bool is_array() const;
  constexpr bool is_string() const;
  constexpr bool is_number() const;
  constexpr bool is_bool() const;
  constexpr bool is_null() const;

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
  JsonNode* parent_;
  JsonNode* sibling_;
  std::vector<JsonNode*> children_;
};

}  // namespace json