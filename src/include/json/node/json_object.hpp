#pragma once

#include <map>
#include <string_view>

#include "json/common/error.hpp"
#include "json/node/json_value.hpp"

namespace json {

class JsonObject {
 public:
  using key_type = std::string;
  using mapped_type = JsonValue;
  using value_type = std::pair<const key_type, mapped_type>;

  JsonObject() = default;
  explicit JsonObject(std::initializer_list<value_type> values)
      : storage_{values} {}

  void append(std::string const& key, JsonValue const& json_value) {
    storage_[key] = json_value;
  }

  friend constexpr bool operator==(JsonObject const& lhs,
                                   JsonObject const& rhs) {
    return lhs.storage_ == rhs.storage_;
  }

  friend constexpr bool operator!=(JsonObject const& lhs,
                                   JsonObject const& rhs) {
    return !(lhs == rhs);
  }

  /// Check if the specified key exists in this object.
  /// \param key The key to search for in this object.
  /// \return `true` if the key exists in this object; otherwise, return false.
  bool has_key(std::string const& key) const {
    return storage_.find(key) != storage_.end();
  }

  JsonValue& operator[](std::string const& key) {
    return const_cast<JsonValue&>(static_cast<const JsonObject&>(*this)[key]);
  }

  /// Search the corresponding value by key in this object.
  ///
  /// \param key The key to search for.
  /// \return A const reference to the associated json value for this `key` if
  ///         this `key` exists in this object; otherwise, an exception will be
  ///         thrown.
  JsonValue const& operator[](std::string const& key) const {
    auto iter = storage_.find(key);

    if (iter == storage_.end()) {
      THROW_ERROR("No such key: " + key);
    }

    return iter->second;
  }

  auto begin() { return storage_.begin(); }
  auto end() { return storage_.end(); }
  auto begin() const { return storage_.begin(); }
  auto end() const { return storage_.end(); }

  auto cbegin() const { return storage_.cbegin(); }
  auto cend() const { return storage_.cend(); }

 private:
  std::map<key_type, mapped_type> storage_;
};

}  // namespace json