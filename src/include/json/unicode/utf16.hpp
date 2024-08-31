#pragma once

#include <cstdint>
#include <utility>

namespace json {
namespace unicode {

class UTF16 {
 public:
  /// 0xd800-0xdc00 encodes the high 10 bits of a pair.
  /// 0xdc00-0xe000 encodes the low 10 bits of a pair.
  /// The value is those 20 bits plus 0x10000.
  static constexpr uint32_t kSurr1 = 0xD800;
  static constexpr uint32_t kSurr2 = 0xDC00;
  static constexpr uint32_t kSurr3 = 0xE000;
  static constexpr uint32_t kSurrSelf = 0x10000;
  static constexpr uint32_t kMaxCodepoint = 0x10FFFF;
  static constexpr uint32_t kReplacementChar = 0xFFFD;

  /// Reports whether the specified Unicode code point can appear in a surrogate
  /// pair.
  ///
  /// \param codepoint The Unicode code point to check.
  /// \return `true` if the code point is within the surrogate pair range
  ///         [U+D800,U+DFFF] otherwise `false`.
  static constexpr bool is_surrogate(uint32_t codepoint) {
    return kSurr1 <= codepoint && codepoint < kSurr3;
  }

  /// Returns the UTF-16 surrogate pair for the given codepoint. If the
  /// codepoint is not a valid Unicode code point or does not need encoding,
  /// returns U+FFFD, U+FFFD.
  ///
  /// \param codepoint The Unicode codepoint to be encoded.
  /// \return A `std::pair<uint32_t, uint32_t>` representing the UTF-16
  ///         surrogate pair. The first element of the pair is the high
  ///         surrogate (surrogate pair first code unit), and the second element
  ///         is the low surrogate.
  static constexpr std::pair<uint32_t, uint32_t> encode(uint32_t codepoint) {
    if (codepoint < kSurrSelf || codepoint > kMaxCodepoint) {
      return {kReplacementChar, kReplacementChar};
    }

    codepoint -= kSurrSelf;

    return {kSurr1 + ((codepoint >> 10) & 0x3FF), kSurr2 + (codepoint & 0x3FF)};
  }

  /// Returns the UTF-16 decoding of a surrogate pair. If the pair is not
  /// a valid UTF-16 surrogate pair, decode returns the Unicode replacement code
  /// point U+FFFD.
  ///
  /// \param codepoint1 The high surrogate code unit. It should be within
  ///                   [0xD800, 0xDC00).
  /// \param codepoint2 The low surrogate code unit. It should be within the
  ///                   [0xDC00, 0xE000).
  /// \return The Unicode codepoint decoded from the surrogate pair. If the
  ///         surrogate pair is invalid, it returns `kReplacementChar`,
  static constexpr uint32_t decode(uint32_t codepoint1, uint32_t codepoint2) {
    if (kSurr1 <= codepoint1 && codepoint1 < kSurr2 && kSurr2 <= codepoint2 &&
        codepoint2 < kSurr3) {
      return (((codepoint1 - kSurr1) << 10) | (codepoint2 - kSurr2)) +
             kSurrSelf;
    }

    return kReplacementChar;
  }
};

}  // namespace unicode
}  // namespace json