#pragma once

#include <type_traits>
#include <variant>

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

/// Grammar
///
/// number = [ minus ] int [ frac ] [ exp ]
/// decimal-point = %x2E       ; .
/// digit1-9 = %x31-39         ; 1-9
/// e = %x65 / %x45            ; e E
/// exp = e [ minus / plus ] 1*DIGIT
/// frac = decimal-point 1*DIGIT
/// int = zero / ( digit1-9 *DIGIT )
/// minus = %x2D               ; -
/// plus = %x2B                ; +
/// zero = %x30                ; 0

namespace json {

class JsonNumber {
 public:
  template <typename T>
    requires std::is_integral_v<T> || std::is_floating_point_v<T>
  explicit JsonNumber(T v) : storage_{v} {}

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
  }

  constexpr void dump(std::ostream& os) const {
    // TODO: fix this.
    // std::visit([&os](auto&& arg) { os << arg; }, storage_);
  }

  friend constexpr bool operator==(JsonNumber const& lhs,
                                   JsonNumber const& rhs);
  friend constexpr bool operator!=(JsonNumber const& lhs,
                                   JsonNumber const& rhs);

 private:
  std::variant<int64_t, double> storage_;
};

constexpr bool operator==(JsonNumber const& lhs, JsonNumber const& rhs) {
  return lhs.storage_ == rhs.storage_;
}

constexpr bool operator!=(JsonNumber const& lhs, JsonNumber const& rhs) {
  return !(lhs == rhs);
}

}  // namespace json