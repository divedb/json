#pragma once

#include <map>
#include <string_view>

#include "json/common/error.hpp"
#include "json/nodes/json_value.hpp"

namespace json {

class JsonObject {
 public:
  using key_type = std::string;
  using mapped_type = JsonValue;
  using value_type = std::pair<const key_type, mapped_type>;

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

  void append(std::string const& key, JsonValue const& json_value) {
    storage_[key] = json_value;
  }

  friend constexpr bool operator==(JsonObject const& lhs,
                                   JsonObject const& rhs) {
    return lhs.storage_ == rhs.storage_;
  }

 private:
  std::map<key_type, mapped_type> storage_;
};

}  // namespace json