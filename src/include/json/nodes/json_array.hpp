#pragma once

#include <vector>

#include "json/nodes/json_value.hpp"

namespace json {

class JsonArray {
 public:
  JsonArray() = default;
  JsonArray(std::initializer_list<JsonValue> values) : storage_(values) {}

  JsonValue& operator[](int index) {
    assert(index >= 0 && index < storage_.size());

    return storage_[index];
  }

  JsonValue const& operator[](int index) const {
    assert(index >= 0 && index < storage_.size());

    return storage_[index];
  }

  void append(JsonValue const& json_value) { storage_.push_back(json_value); }

 private:
  std::vector<JsonValue> storage_;
};

}  // namespace json