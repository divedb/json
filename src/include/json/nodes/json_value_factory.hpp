#pragma once

#include <initializer_list>
#include <type_traits>

#include "json/nodes/json_array.hpp"
#include "json/nodes/json_object.hpp"
#include "json/nodes/json_value.hpp"

namespace json {

class JsonValueFactory {
 public:
  static JsonValue create_null() {
    static JsonValue json_value{JsonNull{}};

    return json_value;
  }

  static JsonValue create_bool(bool b) {
    static JsonValue json_true{true};
    static JsonValue json_false{false};

    return b ? json_true : json_false;
  }

  static JsonValue create_default_bool() { return create_bool(true); }

  template <typename T>
    requires std::is_integral_v<T> || std::is_floating_point_v<T>
  static JsonValue create_number(T v) {
    return JsonValue{v};
  }

  static JsonValue create_default_number() { return create_number(0); }

  static JsonValue create_string(std::string const& s) { return JsonValue{s}; }
  static JsonValue create_default_string() { return create_string(""); }

  template <typename Allocator, typename... JsonValues>
  static JsonValue create_array(Allocator& alloc, JsonValues&&... json_values) {
    void* ptr = alloc.malloc(sizeof(JsonArray));
    auto array = new (ptr) JsonArray();

    (array->append(JsonValue{json_values}), ...);

    return JsonValue{array};
  }

  template <typename Allocator>
  static JsonValue create_default_array(Allocator& alloc) {
    return create_array(alloc);
  }

  template <typename Allocator>
  static JsonValue create_object(Allocator& alloc,
                                 std::initializer_list<std::string> keys,
                                 std::initializer_list<JsonValue> values) {
    void* ptr = alloc.malloc(sizeof(JsonArray));
    auto obj = new (ptr) JsonObject();
    auto kiter = keys.begin();
    auto viter = values.begin();

    while (kiter != keys.end() && viter != values.end()) {
      obj->append(*kiter, *viter);
      kiter++;
      viter++;
    }

    return JsonValue{obj};
  }

  template <typename Allocator>
  static JsonValue create_default_object(Allocator& alloc) {
    return create_object(alloc, {}, {});
  }
};

}  // namespace json