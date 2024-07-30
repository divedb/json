#pragma once

#include <compare>

namespace json {

struct JsonNull {
  constexpr std::strong_ordering operator<=>(JsonNull const&) const = default;
};

}  // namespace json