#pragma once

#include <cerrno>
#include <climits>  // LONG_MAX
#include <cmath>
#include <cstdlib>
#include <type_traits>

namespace json {

template <typename T>
struct Overflow;

template <>
struct Overflow<float> {
  static constexpr auto kMax = HUGE_VAL;
  static constexpr auto kMin = -HUGE_VAL;
};

template <>
struct Overflow<double> {
  static constexpr auto kMax = HUGE_VALF;
  static constexpr auto kMin = -HUGE_VALF;
};

template <>
struct Overflow<long double> {
  static constexpr auto kMax = HUGE_VALL;
  static constexpr auto kMin = -HUGE_VALL;
};

template <>
struct Overflow<long> {
  static constexpr auto kMax = LONG_MAX;
  static constexpr auto kMin = LONG_MIN;
};

template <>
struct Overflow<long long> {
  static constexpr auto kMax = LLONG_MAX;
  static constexpr auto kMin = LLONG_MIN;
};

template <typename T, bool = std::is_floating_point_v<T>>
struct PromotedType;

template <typename T>
struct PromotedType<T, true> {
  using type = T;
};

template <typename T>
struct PromotedType<T, false> {
  using type = typename std::conditional<sizeof(T) <= sizeof(long), long,
                                         long long>::type;
};

class NumberConverter {
 public:
  enum class State : int { kOk, kOverflow, kUnderflow };

  /// @brief Converts a string to a floating point or integral value.
  ///
  /// @tparam T The type to convert the string to. Must be a floating point or
  ///           integral type.
  /// @param str Pointer to the null-terminated byte string to be interpreted
  /// @param str_end Pointer to a pointer to character.
  /// @param base Base of the interpreted integer value
  /// @return 1. If successful, an interger or float value corresponding to the
  ///            contents of str is returned.
  ///         2. If the converted value falls out of range of corresponding
  ///            return type, a range error occurs (setting errno to ERANGE) and
  ///            LONG_MAX, LONG_MIN, LLONG_MAX, LLONG_MIN, HUGE_VAL, HUGE_VALF
  ///            or HUGE_VALL is returned.
  ///         3. If no conversion can be performed, ​0​ is returned.
  template <typename T>
    requires std::is_floating_point_v<T> || std::is_integral_v<T>
  T operator()(char const* str, char** str_end = nullptr, int base = 10) {
    T res;

    if constexpr (std::is_same_v<float, T>) {
      res = strtof(str, str_end);
    } else if constexpr (std::is_same_v<double, T>) {
      res = strtod(str, str_end);
    } else if constexpr (std::is_same_v<long double, T>) {
      res = strtold(str, str_end);
    } else if constexpr (sizeof(T) <= sizeof(long)) {
      res = static_cast<T>(strtol(str, str_end, base));
    } else if constexpr (sizeof(T) <= sizeof(long long)) {
      res = static_cast<T>(strtoll(str, str_end, base));
    }

    if (errno == ERANGE) {
      using U = typename PromotedType<T>::type;

      if (Overflow<U>::kMax == res || Overflow<U>::kMin == res) {
        state_ = State::kOverflow;
      } else {
        state_ = State::kUnderflow;
      }
    }

    return res;
  }

  constexpr State state() const { return state_; }
  constexpr bool is_ok() const { return state_ == State::kOk; }
  constexpr bool is_overflow() const { return state_ == State::kOverflow; }
  constexpr bool is_underflow() const { return state_ == State::kUnderflow; }

  void reset() { state_ = State::kOk; }

 private:
  State state_{State::kOk};
};

}  // namespace json