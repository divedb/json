#pragma once

#include <cstring>  // strlen

namespace json {

// template <u8 target>
// inline constexpr auto is_byte = [](u8 input) { return input == target; };

// inline constexpr bool is_hex(int c) {
//   return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
//          (c >= 'A' && c <= 'F');
// }

// inline constexpr bool is_digit(int c) { return (c >= '0' && c <= '9'); }
// inline constexpr bool is_non_digit(int c) { return !is_digit(c); }
// inline constexpr bool is_exponent(int c) { return c == 'e' || c == 'E'; }

// inline constexpr bool is_space(int c) {
//   return c == 0x20 || c == 0x09 || c == 0x0a || c == 0x0d;
// }

// inline constexpr bool is_ascii(int c) { return c >= 0 && c <= 0xff; }

// Parse the unicode between first and last.
// Example:
// s = "1234"
// cp = unicode_to_codepoint(s, s + strlen(s))
// EXPECT_EQ(0x1234, cp)
// template <typename InputIt>
// inline i32 unicode_to_codepoint(InputIt first, InputIt last) {
//   std::string tmp(first, last);
//   std::istringstream iss(tmp);

//   i32 n = 0;
//   iss >> std::hex >> n;

//   return n;
// }

/// @brief Computes the smallest power of 2 greater than or equal to the given
///        value.
///
/// @param v The value to compute the next power of 2 for.
/// @return The smallest power of 2 that is greater than or equal to the input
///         value.
constexpr unsigned int next_power_of_2(unsigned int v) {
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v++;

  return v;
}

template <typename InputIt>
constexpr bool is_float(InputIt first, InputIt last) {
  return std::find_if(first, last, [](char ch) {
           return ch == '.' || ch == 'e' || ch == 'E';
         }) != last;
}

bool is_float(char const* cstr) { return is_float(cstr, cstr + strlen(cstr)); }

[[noreturn]] inline void unreachable() {
  // Uses compiler specific extensions if possible.
  // Even if no extension is used, undefined behavior is still raised by
  // an empty function body and the noreturn attribute.
#if defined(_MSC_VER) && !defined(__clang__)  // MSVC
  __assume(false);
#else  // GCC, Clang
  __builtin_unreachable();
#endif
}

}  // namespace json