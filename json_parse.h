#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iterator>
#include <string>

namespace json {

static inline bool is_exp(int v) { return v == 'e' || v == 'E'; }
static inline bool is_decimal_point(int v) { return v == '.'; }
static inline bool is_minus(int v) { return v == '-'; }
static inline bool is_plus(int v) { return v == '+'; }
static inline bool is_digit(int v) { return v >= '0' && v <= '9'; }
static inline bool is_zero(int v) { return v == '0'; }

enum JsonType : uint8_t { kInvalid, kString, kNumber, kBool, kNull, kObject, kArray };

using BigInteger = long long;
using String = std::string;

struct Number {
 public:
  Number() = default;
  explicit Number(BigInteger value) : is_integer_{true}, value_{value} {}
  explicit Number(double value) : is_integer_{false}, value_{value} {}

  bool is_integer() const { return is_integer_; }

  template <typename T>
  T get() const {
    if constexpr (std::is_same_v<T, BigInteger>) {
      return std::get<BigInteger>(value_);
    }

    if constexpr (std::is_same_v<T, double>) {
      return std::get<double>(value_);
    }
  }

  friend bool operator==(const Number& lhs, const Number& rhs) {
    return lhs.is_integer_ == rhs.is_integer_ && lhs.value_ == rhs.value_;
  }

  friend bool operator!=(const Number& lhs, const Number& rhs) { return !(lhs == rhs); }

 private:
  bool is_integer_;
  std::variant<BigInteger, double> value_;
};

class JsonValue {
 public:
  JsonValue() = default;

  explicit JsonValue(Number num) : type_{kNumber}, value_{num} {}

  bool is_valid() const { return type_ != JsonType::kInvalid; }
  JsonType type() const { return type_; }
  void set_type(JsonType type) { type_ = type; }

  template <typename T>
  T get() const {
    if constexpr (std::is_same_v<T, Number>) {
      return std::get<Number>(value_);
    }

    if constexpr (std::is_same_v<T, String>) {
      return std::get<String>(value_);
    }
  }

  void print() const {
    switch (type_) {
      case kInvalid:
        std::cout << "invalid\n";
        break;

      case kString:
        std::cout << "string\n";
        break;

      case kNumber:
        std::cout << "number\n";
        break;

      case kBool:
        std::cout << "bool\n";
        break;

      case kNull:
        std::cout << "null\n";
        break;

      default:
        std::cout << "default\n";
        break;
    }
  }

 private:
  JsonType type_{JsonType::kInvalid};
  std::variant<String, Number> value_;
};

enum State : uint8_t {
  kOk,
  kInvalidNumber,
  kInvalidString,
  kInvalidBool,
  kInvalidNull,
  kInvalidArray,
  kInvalidObject,
  kEOF
};

struct ParseState {
  bool is_ok() const { return state == State::kOk; }

  State state{State::kOk};
  int lineno{0};
};

template <typename InputIter>
bool parse_int(InputIter& first, InputIter last, ParseState& ps) {
  bool has_zero = has_one(first, last, is_zero, ps);

  if (ps.state == State::kEOF) {
    return false;
  }

  bool has_digit = has_one_or_more(first, last, is_digit, ps);

  // case 1: other digits follows 0
  // case 2: no digits
  if ((has_zero && has_digit) || (!has_zero && !has_digit)) {
    ps.state = State::kInvalidNumber;

    return false;
  }

  return true;
}

// Numeric values that cannot be represented in the grammar below (such
// as Infinity and NaN) are not permitted.
// number = [ minus ] int [ frac ] [ exp ]
//
// decimal-point = %x2E                         ; .
// digit1-9      = %x31-39                      ; 1-9
// e             = %x65/%x45                    ; e-E
// exp           = e [ minus / plus ] 1*DIGIT   ;
// frac          = decimal-point 1*DIGIT
// int           = zero / ( digit1-9 *DIGIT )
// minus         = %x2D                         ; -
// plus          = %x2B                         ; +
// zero          = %x30                         ; 0
template <typename InputIter>
bool parse_number(InputIter& first, InputIter last, ParseState& ps, JsonValue& out) {
  using VT = typename std::iterator_traits<InputIter>::value_type;

  InputIter init = first;

  bool has_minus = has_one(first, last, is_minus, ps);

  if (!parse_int(first, last, ps)) {
    return false;
  }

  bool has_frac = has_one(first, last, is_decimal_point, ps);

  if (has_frac && !has_one_or_more(first, last, is_digit, ps)) {
    return false;
  }

  bool has_exp = has_one(first, last, is_exp, ps);
  auto is_minus_or_plus = [](VT v) { return is_minus(v) || is_plus(v); };

  if (has_exp && (!has_one(first, last, is_minus_or_plus, ps) || !has_one_or_more(first, last, is_digit, ps))) {
    return false;
  }

  if (!has_frac) {
    BigInteger num = strtol(init, (char**)NULL, 10);
    out = JsonValue{Number(num)};
  } else {
    double v = strtod(init, NULL);
    out = JsonValue{Number(v)};
  }

  return true;
}

ParseState parse_number(const char* input, JsonValue& json_value) {
  ParseState ps;
  std::size_t size = strlen(input);

  if (!parse_number(input, input + size, ps, json_value)) {
    ps.state = State::kInvalidNumber;
  } else {
    ps.state = State::kOk;
  }

  return ps;
}

}  // namespace json