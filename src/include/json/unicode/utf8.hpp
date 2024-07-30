#pragma once

#include <cassert>
#include <cstdint>

namespace json {

class UTF8 {
 public:
  static constexpr int kT1 = 0b00000000;
  static constexpr int kTx = 0b10000000;
  static constexpr int kT2 = 0b11000000;
  static constexpr int kT3 = 0b11100000;
  static constexpr int kT4 = 0b11110000;
  static constexpr int kT5 = 0b11111000;

  static constexpr int kMaskx = 0b00111111;
  static constexpr int kMask2 = 0b00011111;
  static constexpr int kMask3 = 0b00001111;
  static constexpr int kMask4 = 0b00000111;

  static constexpr int kRune1Max = (1 << 7) - 1;
  static constexpr int kRune2Max = (1 << 11) - 1;
  static constexpr int kRune3Max = (1 << 16) - 1;
  static constexpr int kMaxRune = 0x10FFFF;
  static constexpr int kSurrogateMin = 0xD800;
  static constexpr int kSurrogateMax = 0xDFFF;

  /// @brief Writes into output iterator (which must be large enough) the UTF-8
  ///        encoding of the rune.
  ///
  /// @tparam OutputIt The type of the output iterator.
  /// @param d_first The output iterator where the UTF-8 bytes will be written.
  /// @param rune The Unicode code point (rune) to encode.
  /// @return The number of bytes written to the output iterator.
  template <typename OutputIt>
  static int encode(OutputIt d_first, int32_t rune) {
    auto u = static_cast<uint32_t>(rune);

    assert(!(u > kMaxRune || (u >= kSurrogateMin && u <= kSurrogateMax)));

    if (u <= kRune1Max) {
      *d_first = rune;

      return 1;
    }

    if (u <= kRune2Max) {
      *d_first++ = kT2 | ((rune >> 6) & 0xFF);
      *d_first = kTx | ((rune & 0xFF) & kMaskx);

      return 2;
    }

    if (u <= kRune3Max) {
      *d_first++ = kT3 | ((u >> 12) & 0xFF);
      *d_first++ = kTx | (((u >> 6) & 0xFF) & kMaskx);
      *d_first = kTx | ((u & 0xFF) & kMaskx);

      return 3;
    }

    *d_first++ = kT4 | ((u >> 18) & 0xFF);
    *d_first++ = kTx | (((u >> 12) & 0xFF) & kMaskx);
    *d_first++ = kTx | (((u >> 6) & 0xFF) & kMaskx);
    *d_first = kTx | ((u & 0xFF) & kMaskx);

    return 4;
  }
};

}  // namespace json