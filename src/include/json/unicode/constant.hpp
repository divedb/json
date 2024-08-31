#pragma once

namespace json::unicode {

inline constexpr unsigned kMaxCodepoint = 0x10FFFF;

inline constexpr bool is_valid_unicode(uint32_t codepoint) {
  return codepoint <= kMaxCodepoint;
}

}  // namespace json::unicode