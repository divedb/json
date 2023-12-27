#pragma once

#include <cstdint>
#include <string>

namespace json {

using JsonChar = char;
using Buffer = std::basic_string<JsonChar>;
using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;

using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;

}  // namespace json