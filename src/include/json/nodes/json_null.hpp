#pragma once

#include <compare>
#include <iostream>

namespace json {

struct JsonNull {
  constexpr std::strong_ordering operator<=>(JsonNull const&) const = default;

  friend std::ostream& operator<<(std::ostream& os, JsonNull const&) {
    return os << "null";
  }
};

}  // namespace json