#pragma once

#include <algorithm>
#include <cctype>   // isxdigit
#include <cstring>  // strlen

namespace json {

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

/// @brief Converts a hexadecimal character to its integer value.
///
/// @param ch The hexadecimal character to be converted.
/// @return The integer value corresponding to the hexadecimal character.
inline int hex_char_to_int(int ch) {
  assert(std::isxdigit(ch));

  if (std::isdigit(ch)) {
    return ch - '0';
  }

  return std::tolower(ch) - 'a' + 10;
}

}  // namespace json