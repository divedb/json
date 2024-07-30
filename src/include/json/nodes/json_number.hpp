#pragma once

#include <iostream>
#include <type_traits>
#include <variant>

#include "json/common/util.hpp"

/// RFC7159
/// The representation of numbers is similar to that used in most
/// programming languages. A number is represented in base 10 using
/// decimal digits.  It contains an integer component that may be
/// prefixed with an optional minus sign, which may be followed by a
/// fraction part and/or an exponent part.  Leading zeros are not
/// allowed.
///
/// A fraction part is a decimal point followed by one or more digits.
/// An exponent part begins with the letter E in upper or lower case,
/// which may be followed by a plus or minus sign. The E and optional
/// sign are followed by one or more digits.
///
/// Numeric values that cannot be represented in the grammar below (such
/// as Infinity and NaN) are not permitted.

namespace json {

class JsonNumber {
 public:
  using Storage = std::variant<int64_t, double>;

  constexpr JsonNumber() = default;

  template <typename T>
    requires std::is_integral_v<T> || std::is_floating_point_v<T>
  constexpr explicit JsonNumber(T v) : storage_{v} {}
  constexpr explicit JsonNumber(Storage storage) : storage_{storage} {}

  constexpr bool is_integer() const { return storage_.index() == 0; }

  template <typename T>
  T get() const {
    static_assert(std::is_floating_point_v<T> || std::is_integral_v<T>,
                  "Unsupported type");

    if constexpr (std::is_integral_v<T>) {
      return static_cast<T>(std::get<0>(storage_));
    }

    if constexpr (std::is_floating_point_v<T>) {
      return static_cast<T>(std::get<1>(storage_));
    }

    unreachable();
  }

  friend std::ostream& operator<<(std::ostream& os,
                                  JsonNumber const& json_num) {
    std::visit([&os](auto&& arg) { os << arg; }, json_num.storage_);

    return os;
  }

  friend constexpr bool operator==(JsonNumber const& lhs,
                                   JsonNumber const& rhs);
  friend constexpr bool operator!=(JsonNumber const& lhs,
                                   JsonNumber const& rhs);

 private:
  Storage storage_;
};

constexpr bool operator==(JsonNumber const& lhs, JsonNumber const& rhs) {
  return lhs.storage_ == rhs.storage_;
}

constexpr bool operator!=(JsonNumber const& lhs, JsonNumber const& rhs) {
  return !(lhs == rhs);
}

}  // namespace json