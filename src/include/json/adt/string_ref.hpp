//===- StringRef.h - Constant String Reference Wrapper ----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace json {

/// StringRef - Represent a constant reference to a string, i.e. a character
/// array and a length, which need not be null terminated.
///
/// This class does not own the string data, it is expected to be used in
/// situations where the character data resides in some other buffer, whose
/// lifetime extends past that of the StringRef. For this reason, it is not in
/// general safe to store a StringRef.
class StringRef {
 public:
  static constexpr size_t npos = ~size_t(0);

  using iterator = const char*;
  using const_iterator = const char*;
  using size_type = size_t;

 private:
  /// The start of the string, in an external buffer.
  const char* data_ = nullptr;

  /// The length of the string.
  size_t length_ = 0;

  // Workaround memcmp issue with null pointers (undefined behavior)
  // by providing a specialized version
  static int compare_memory(const char* lhs, const char* rhs, size_t length) {
    if (length == 0) {
      return 0;
    }

    return ::memcmp(lhs, rhs, length);
  }

 public:
  /// @name Constructors
  /// @{

  /// Construct an empty string ref.
  /*implicit*/ StringRef() = default;

  /// Disable conversion from nullptr.  This prevents things like
  /// if (S == nullptr)
  StringRef(std::nullptr_t) = delete;

  /// Construct a string ref from a cstring.
  /*implicit*/ constexpr StringRef(const char* str)
      : data_(str),
        length_(str ?
  // GCC 7 doesn't have constexpr char_traits. Fall back to __builtin_strlen.
#if defined(_GLIBCXX_RELEASE) && _GLIBCXX_RELEASE < 8
                    __builtin_strlen(Str)
#else
                    std::char_traits<char>::length(str)
#endif
                    : 0) {
  }

  /// Construct a string ref from a pointer and length.
  /*implicit*/ constexpr StringRef(const char* data, size_t length)
      : data_(data), length_(length) {}

  /// Construct a string ref from an std::string.
  /*implicit*/ StringRef(const std::string& str)
      : data_(str.data()), length_(str.length()) {}

  /// Construct a string ref from an std::string_view.
  /*implicit*/ constexpr StringRef(std::string_view str)
      : data_(str.data()), length_(str.size()) {}

  /// @}
  /// @name Iterators
  /// @{
  iterator begin() const { return data_; }
  iterator end() const { return data_ + length_; }

  iterator begin() const { return data_; }

  iterator end() const { return data_ + length_; }

  const unsigned char* bytes_begin() const {
    return reinterpret_cast<const unsigned char*>(begin());
  }
  const unsigned char* bytes_end() const {
    return reinterpret_cast<const unsigned char*>(end());
  }
  iterator_range<const unsigned char*> bytes() const {
    return make_range(bytes_begin(), bytes_end());
  }
};

}  // namespace json