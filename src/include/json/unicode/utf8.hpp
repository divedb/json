#pragma once

#include <cstdint>

#include "json/unicode/constant.hpp"

namespace json {
namespace unicode {

class UTF8 {
 public:
  using CharT = unsigned char;

  /// Numbers fundamental to the encoding.
  static constexpr const unsigned kRuneError = 0xFFFD;
  static constexpr const unsigned kRuneSelf = 0x80;
  static constexpr const unsigned kUTFMax = 4;

  /// Code points in the surrogate range are not valid for UTF-8.
  static constexpr const unsigned kSurrogateMin = 0xD800;
  static constexpr const unsigned kSurrogateMax = 0xDFFF;

  static constexpr const CharT kT1 = 0b00000000;
  static constexpr const CharT kTx = 0b10000000;
  static constexpr const CharT kT2 = 0b11000000;
  static constexpr const CharT kT3 = 0b11100000;
  static constexpr const CharT kT4 = 0b11110000;
  static constexpr const CharT kT5 = 0b11111000;

  static constexpr const CharT kMaskx = 0b00111111;
  static constexpr const CharT kMask2 = 0b00011111;
  static constexpr const CharT kMask3 = 0b00001111;
  static constexpr const CharT kMask4 = 0b00000111;

  static constexpr const unsigned kRune1Max = (1 << 7) - 1;
  static constexpr const unsigned kRune2Max = (1 << 11) - 1;
  static constexpr const unsigned kRune3Max = (1 << 16) - 1;

  /// The default lowest and highest continuation byte.
  static constexpr const CharT kLocb = 0b10000000;
  static constexpr const CharT kHicb = 0b10111111;

  static constexpr const CharT kXx = 0xF1;  /// Invalid: size 1
  static constexpr const CharT kAs = 0xF0;  /// kAsCII: size 1
  static constexpr const CharT kS1 = 0x02;  /// Accept 0, size 2
  static constexpr const CharT kS2 = 0x13;  /// Accept 1, size 3
  static constexpr const CharT kS3 = 0x03;  /// Accept 0, size 3
  static constexpr const CharT kS4 = 0x23;  /// Accept 2, size 3
  static constexpr const CharT kS5 = 0x34;  /// Accept 3, size 4
  static constexpr const CharT kS6 = 0x04;  /// Accept 0, size 4
  static constexpr const CharT kS7 = 0x44;  /// Accept 4, size 4

  struct AcceptRange {
    uint8_t lo;  /// Lowest value for second byte.
    uint8_t hi;  /// Highest value for second byte.
  };

  static constexpr const AcceptRange kAcceptedRange[]{
      {kLocb, kHicb},  //
      {0xA0, kHicb},   /// This corresponds to 0x800
      {kLocb, 0x9F},   //
      {0x90, kHicb},   //
      {kLocb, 0x8F},   //
  };

  /// First is information about the first byte in a UTF-8 sequence.
  /// Table 3.1B. Legal UTF-8 Byte Sequences.
  /// Code Points       | 1st Byte  | 2nd Byte  | 3rd Byte  | 4th Byte  |
  /// U+0000..U+007F    | 00..7F    |           |           |           |
  /// U+0080..U+07FF    | C2..DF    | 80..BF    |           |           |
  /// U+0080..U+0FFF    | E0        | A0..BF    | 80..BF    |           |
  /// U+1000..U+FFFF    | E1..EF    | B0..BF    | 80..BF    |           |
  /// U+10000..U+3FFFF  | F0        | 90..BF    | 80..BF    | 80..BF    |
  /// U+40000..U+FFFFF  | F1..F3    | 80..BF    | 80..BF    | 80..BF    |
  /// U+100000..U+10FFFF| F4        | 80..BF    | 80..BF    | 80..BF    |
  static constexpr const uint8_t kFirst[256]{
      ///    1    2    3    4    5    6    7    8    9    A    B    C    D    E
      ///    F
      kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs,
      kAs,  // 0x00-0x0F
      kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs,
      kAs,  // 0x10-0x1F
      kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs,
      kAs,  // 0x20-0x2F
      kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs,
      kAs,  // 0x30-0x3F
      kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs,
      kAs,  // 0x40-0x4F
      kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs,
      kAs,  // 0x50-0x5F
      kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs,
      kAs,  // 0x60-0x6F
      kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs, kAs,
      kAs,  // 0x70-0x7F
      ///    1    2    3    4    5    6    7    8    9    A    B    C    D    E
      ///    F
      kXx, kXx, kXx, kXx, kXx, kXx, kXx, kXx, kXx, kXx, kXx, kXx, kXx, kXx, kXx,
      kXx,  // 0x80-0x8F
      kXx, kXx, kXx, kXx, kXx, kXx, kXx, kXx, kXx, kXx, kXx, kXx, kXx, kXx, kXx,
      kXx,  // 0x90-0x9F
      kXx, kXx, kXx, kXx, kXx, kXx, kXx, kXx, kXx, kXx, kXx, kXx, kXx, kXx, kXx,
      kXx,  // 0xA0-0xAF
      kXx, kXx, kXx, kXx, kXx, kXx, kXx, kXx, kXx, kXx, kXx, kXx, kXx, kXx, kXx,
      kXx,  // 0xB0-0xBF
      kXx, kXx, kS1, kS1, kS1, kS1, kS1, kS1, kS1, kS1, kS1, kS1, kS1, kS1, kS1,
      kS1,  // 0xC0-0xCF
      kS1, kS1, kS1, kS1, kS1, kS1, kS1, kS1, kS1, kS1, kS1, kS1, kS1, kS1, kS1,
      kS1,  // 0xD0-0xDF
      kS2, kS3, kS3, kS3, kS3, kS3, kS3, kS3, kS3, kS3, kS3, kS3, kS3, kS4, kS3,
      kS3,  // 0xE0-0xEF
      kS5, kS6, kS6, kS6, kS7, kXx, kXx, kXx, kXx, kXx, kXx, kXx, kXx, kXx, kXx,
      kXx,  // 0xF0-0xFF
  };

  template <typename OutputIt>
  static int encode(OutputIt iter, unsigned int codepoint) {
    assert(!(codepoint > kMaxCodepoint ||
             (codepoint >= kSurrogateMin && kSurrogateMax)));

    if (codepoint <= kRune1Max) {
      *iter = static_cast<CharT>(codepoint);

      return 1;
    } else if (codepoint <= kRune2Max) {
      *iter++ = kT2 | static_cast<CharT>(codepoint >> 6);
      *iter = kTx | (static_cast<CharT>(codepoint) & kMaskx);

      return 2;
    } else if (codepoint <= kRune3Max) {
      *iter++ = kT3 | static_cast<CharT>(codepoint >> 12);
      *iter++ = kTx | (static_cast<CharT>(codepoint >> 6) & kMaskx);
      *iter = kTx | (static_cast<CharT>(codepoint) & kMaskx);

      return 3;
    } else {
      *iter++ = kT4 | static_cast<CharT>(codepoint >> 18);
      *iter++ = kTx | (static_cast<CharT>(codepoint >> 12) & kMaskx);
      *iter++ = kTx | (static_cast<CharT>(codepoint >> 6) & kMaskx);
      *iter = kTx | (static_cast<CharT>(codepoint) & kMaskx);

      return 4;
    }
  }

  template <typename InputIt>
  static int decode(InputIt& iter, unsigned int& codepoint) {
    CharT b0 = *iter++;
    auto x = kFirst[b0];

    if (x >= kAs) {
      // The following code simulates an additional check for x == xx and
      // handling the ASCII and invalid cases accordingly. This mask-and-or
      // approach prevents an additional branch.
      unsigned mask = (int32_t(x) << 31) >> 31;  // Create 0x0000 or 0xFFFF.
      codepoint = (b0 & (~mask)) | (kRuneError & mask);

      return 1;
    }

    auto sz = static_cast<int>(x & 7);

    // if (n < sz) {
    //   return 1;
    // }
    CharT b1 = *iter++;
    auto accept = kAcceptedRange[x >> 4];

    /// If the input stream is 110xxxxx, 10xxxxxx
    /// the second byte must be within [10000000, 10111111]
    if (b1 < accept.lo || accept.hi < b1) {
      codepoint = kRuneError;

      return 1;
    }

    if (sz <= 2) {
      codepoint = (static_cast<unsigned>(b0 & kMask2) << 6) | (b1 & kMaskx);

      return 2;
    }

    CharT b2 = *iter++;

    if (b1 < kLocb || kHicb < b2) {
      codepoint = kRuneError;

      return 1;
    }

    if (sz <= 3) {
      codepoint = (static_cast<unsigned>(b0 & kMask3) << 12) |
                  (static_cast<unsigned>(b1 & kMaskx) << 6) |
                  static_cast<unsigned>(b2 & kMaskx);

      return 3;
    }

    CharT b3 = *iter++;

    if (b3 < kLocb || kHicb < b3) {
      codepoint = kRuneError;

      return 1;
    }

    codepoint = (static_cast<unsigned>(b0 & kMask4) << 18) |
                (static_cast<unsigned>(b1 & kMaskx) << 12) |
                (static_cast<unsigned>(b2 & kMaskx) << 6) |
                static_cast<unsigned>(b3 & kMaskx);

    return 4;
  }
};

}  // namespace unicode
}  // namespace json