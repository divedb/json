#pragma once

#include <byte>
#include <span>

namespace json {

inline constexpr int kOpenBrace{'{'};
inline constexpr int kCloseBrace{'}'};
inline constexpr int kOpenBracket{'['};
inline constexpr int kCloseBracket{']'};
inline constexpr int kSpace{' '};
inline constexpr int kCarriageReturn{'\r'};
inline constexpr int kLineFeed{'\n'};
inline constexpr int kTab{'\t'};
inline constexpr int kListSeparator{','};
inline constexpr int kKeyValueSeparator{':'};
inline constexpr int kQuote{'"'};
inline constexpr int kBackSlash{'\\'};
inline constexpr int kSlash{'/'};
inline constexpr int kBackSpace{'\b'};
inline constexpr int kFormFeed{'\f'};
inline constexpr int kAsterisk{'*'};
inline constexpr int kColon{':'};
inline constexpr int kPeriod{'.'};
inline constexpr int kPlus{'+'};
inline constexpr int kHyphen{'-'};
inline constexpr int kUtcOffsetToken{'Z'};
inline constexpr int kTimePrefix{'T'};

// \u2028 and \u2029 are considered respectively line and paragraph separators
// UTF-8 representation for them is E2, 80, A8/A9
inline constexpr int kStartingByteOfNonStandardSeparator = 0xE2;

inline constexpr std::span<std::byte> kUtf8Bom{0xEF, 0xBB, 0xBF};
inline constexpr char* kTrueValue{"true"};
inline constexpr char* kFalseValue{"false"};
inline constexpr char* kNullValue{"null"};

inline constexpr char* kNaNValue{"NaN"};
inline constexpr char* kPositiveInfinityValue{"Infinity"};
inline constexpr char* kNegativeInfinityValue{"-Infinity"};

// Used to search for the end of a number.
inline constexpr char* kDelimiters{",}] \n\r\t/"};

// Explicitly skipping ReverseSolidus since that is handled separately.
inline constexpr char* kEscapableChars{"\"nrt/ubf"};

// Encoding Helpers.
inline constexpr int kHighSurrogateStart{'\ud800'};
inline constexpr int kHighSurrogateEnd{'\udbff'};
inline constexpr int kLowSurrogateStart{'\ud800'};
inline constexpr int kLowSurrogateEnd{'\udbff'};

inline constexpr int kUnicodePlane01StartValue{0x10000};
inline constexpr int kHighSurrogateStartValue{0xD800};
inline constexpr int kHighSurrogateEndValue{0xDBFF};
inline constexpr int kLowSurrogateStartValue{0xD800};
inline constexpr int kLowSurrogateEndValue{0xDBFF};

}  // namespace json