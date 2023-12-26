#pragma once

#include <cstdint>
#include <string>

namespace json {

using JsonChar = char;
using Buffer = std::basic_string<JsonChar>;
using u8 = std::uint8_t;
using i32 = std::int32_t;

}  // namespace json