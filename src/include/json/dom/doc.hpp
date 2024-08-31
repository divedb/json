#pragma once

#include <memory>
#include <string_view>

#include "json/common/memory_context.hpp"
#include "json/parser/json_parser.hpp"

namespace json {

class Document {
 public:
  Document() = default;

  void parse(std::string_view data) {
    auto [value, err] = JsonParser::parse(data.begin(), data.end(), mem_ctx_);

    if (err == ErrorCode::kOk) {
      root_ = value;
    }
  }

  constexpr bool is_object() const { return root_.is_object(); }

  /// Check if this document contains the specified key.
  ///
  /// \param key The key to search for.
  /// \return `true` if the key exists; otherwise, return `false`.
  bool has_member(char const* key) const {
    assert(is_object());

    auto pobj = root_.as_object();

    return pobj->has_key(key);
  }

  /// Access the value associated with the specified key in this object,
  /// allowing modification of the value.
  ///
  /// \param key The key to search for.
  /// \return A reference to the associated JSON value for this `key`.
  JsonValue& operator[](char const* key) {
    assert(is_object());

    auto pobj = root_.as_object();

    return pobj->operator[](key);
  }

  /// Access the value associated with the specified key in this object,
  /// without allowing modification of the value.
  ///
  /// \param key The key to search for.
  /// \return A const reference to the associated JSON value for this `key`.
  JsonValue const& operator[](char const* key) const {
    assert(is_object());

    auto pobj = root_.as_object();

    return pobj->operator[](key);
  }

  // Debug.
  JsonValue const& root() const { return root_; }

 private:
  JsonValue root_{};
  MemoryContext mem_ctx_;
};

}  // namespace json