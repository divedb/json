#pragma once

#include <map>
#include <string_view>

#include "json/error.hpp"
#include "json/nodes/json_value.hpp"

namespace json {

class JsonObject {
 public:
  using value_type = std::pair<const std::string, JsonValue>;

  JsonObject() = default;
  explicit JsonObject(std::initializer_list<value_type> values)
      : storage_{values} {}

  JsonValue& operator[](std::string const& key) {
    return const_cast<JsonValue&>(static_cast<const JsonObject&>(*this)[key]);
  }

  JsonValue const& operator[](std::string const& key) const {
    auto iter = storage_.find(key);

    if (iter == storage_.end()) {
      THROW_ERROR("No such key: " + key);
    }

    return iter->second;
  }

 private:
  std::map<std::string, JsonValue> storage_;
};

}  // namespace json