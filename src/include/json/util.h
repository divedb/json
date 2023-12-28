#pragma once

#include "json/types.h"

namespace json {

template <u8 target>
inline constexpr auto is_byte = [](u8 input) { return input == target; };

inline constexpr bool is_hex(int c) {
  return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
         (c >= 'A' && c <= 'F');
}

inline constexpr bool is_digit(int c) { return (c >= '0' && c <= '9'); }

inline constexpr bool is_exponent(int c) { return c == 'e' || c == 'E'; }

unsigned int next_power_of_2(unsigned int n) {
  if (n && !(n & (n - 1))) {
    return n;
  }

  unsigned int p = 1;
  while (p < n) {
    p <<= 1;
  }

  return p;
}

}  // namespace json