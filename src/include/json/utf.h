#pragma once

#include <stdexcept>
#include <string>

#include "json/types.h"

namespace json {

enum class ByteOrder : std::int8_t { kLittleEndian, kBigEndian };

// 0xd800-0xdc00 encodes the high 10 bits of a pair.
// 0xdc00-0xe000 encodes the low 10 bits of a pair.
// the value is those 20 bits plus 0x10000.
inline constexpr const i32 kSurr1 = 0xd800;
inline constexpr const i32 kSurr2 = 0xdc00;
inline constexpr const i32 kSurr3 = 0xe000;
inline constexpr const i32 kSurrSelf = 0x10000;
inline constexpr const i32 kSizeOfSurr = 2;

inline constexpr bool is_high_surrogate(i32 v) {
  return v >= kSurr1 && v < kSurr2;
}
inline constexpr bool is_low_surrogate(i32 v) {
  return v >= kSurr2 && v < kSurr3;
}
inline constexpr bool is_surrogate(i32 v) { return v >= kSurr1 && v < kSurr3; }

class UTF8 {
 public:
  // Maximum valid Unicode code point
  constexpr static const int kMaxRune = 0x0010ffff;

  // Maximum number of bytes of a UTF-8 encoded Unicode character
  constexpr static const int kUTFMax = 4;

  constexpr static const int kRune1Max = (1 << 7) - 1;
  constexpr static const int kRune2Max = (1 << 11) - 1;
  constexpr static const int kRune3Max = (1 << 16) - 1;

  constexpr static const u8 kT2 = 0b11000000;
  constexpr static const u8 kT3 = 0b11100000;
  constexpr static const u8 kT4 = 0b11110000;
  constexpr static const u8 kTx = 0b10000000;

  constexpr static const u8 kMaskX = 0b00111111;

  // Writes into buffer the UTF-8 encoding of the rune.
  // If the rune if out of range, it throws invalid_argument exception.
  static Buffer encode(i32 r) {
    Buffer buffer;

    if (r <= kRune1Max) {
      buffer.push_back(static_cast<u8>(r));
    } else if (r <= kRune2Max) {
      buffer.push_back(kT2 | static_cast<u8>(r >> 6));
      buffer.push_back(kTx | (static_cast<u8>(r) & kMaskX));
    } else if (r > kMaxRune || is_surrogate(r)) {
      std::string error =
          "[UTF8::encode]: invalid unicode: " + std::to_string(r);
      throw std::invalid_argument(error);
    } else if (r <= kRune3Max) {
      buffer.push_back(kT3 | static_cast<u8>(r >> 12));
      buffer.push_back(kTx | (static_cast<u8>(r >> 6) & kMaskX));
      buffer.push_back(kTx | (static_cast<u8>(r) & kMaskX));
    } else {
      buffer.push_back(kT4 | static_cast<u8>(r >> 18));
      buffer.push_back(kTx | (static_cast<u8>(r >> 12) & kMaskX));
      buffer.push_back(kTx | (static_cast<u8>(r >> 6) & kMaskX));
      buffer.push_back(kTx | (static_cast<u8>(r) & kMaskX));
    }

    return buffer;
  }
};

class UTF16 {
 public:
  constexpr static i32 decode(i32 r1, i32 r2) {
    if (!is_high_surrogate(r1) || !is_low_surrogate(r2)) {
      std::string error = "[UTF16::encode]: invalid unicode: [" +
                          std::to_string(r1) + "," + std::to_string(r2) + "]";
      throw std::invalid_argument(error);
    }

    return (((r1 - kSurr1) << 10) | (r2 - kSurr2)) + kSurrSelf;
  }
};

}  // namespace json