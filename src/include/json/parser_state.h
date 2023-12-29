#pragma once

#include <algorithm>
#include <deque>
#include <iostream>

#include "json/error.h"
#include "json/types.h"
#include "json/util.h"

namespace json {

enum class Status : u8 { kError, kEOF, kOk };

template <typename InputIt>
class ParserState {
 public:
  using value_type = typename std::iterator_traits<InputIt>::value_type;
  constexpr static const value_type kSentinel{};
  // TODO(gc): do we need to utilize the SSO?
  constexpr static const int kBufSize = 256;

  // Constructor.
  constexpr ParserState(InputIt first, InputIt last,
                        Status status = Status::kOk)
      : line_number(0),
        pipes(0),
        status(status),
        cache_(kSentinel),
        cursor(0),
        buf(kBufSize, 0),
        first_(first),
        last_(last),
  {}

  constexpr void put(u8 b) {
    assert(cache_ == kSentinel);
    cache_ = b;
  }

  constexpr u8 back() const {
    assert(cursor != 0);

    return buf[cursor - 1];
  }

  constexpr u8 pop_back() {
    assert(cursor != 0);
    cursor--;

    return buf[cursor];
  }

  constexpr u8 next() {
    if (cache_ != kSentinel) {
      auto res = cache_;
      cache_ = kSentinel;

      return res;
    }

    return *first_++;
  }

  constexpr bool has_next() const noexcept {
    return cache_ != kSentinel || first_ != last_;
  }

  constexpr bool is_ok() const noexcept { return status == Status::kOk; }

  // Get the consumed bytes when data is piped.
  Buffer buffer() const noexcept { return buf.substr(0, cursor); }

  int line_number;
  int pipes;
  Status status;
  Error error;
  int cursor;
  Buffer buf;

 private:
  value_type cache_;
  InputIt first_;
  InputIt last_;
};

#define CHECK_PARSER_STATE(state) \
  do {                            \
    if (!state.is_ok()) {         \
      return state;               \
    }                             \
  } while (0)

#define IF_EOF_RETURN(state, exp) \
  do {                            \
    if (!state.has_next()) {      \
      return (exp);               \
    }                             \
  } while (0)

template <std::size_t N, typename InputIt, typename Predicate>
inline constexpr ParserState<InputIt> has_fixed(ParserState<InputIt> state,
                                                Predicate&& p) {
  CHECK_PARSER_STATE(state);

  std::size_t i;

  for (i = 0; i < N && state.has_next(); ++i) {
    u8 b = state.next();

    if (!std::forward<Predicate>(p)(b)) {
      state.put(b);
      state.status = Status::kError;
      break;
    } else {
      state.buf[state.cursor++] = b;
    }
  }

  if (i < N && state.status != Status::kError) {
    state.status = Status::kEOF;
  }

  if (state.is_ok()) {
    state.pipes++;
  }

  return state;
}

template <typename InputIt, typename Predicate>
inline constexpr ParserState<InputIt> has_one(ParserState<InputIt> state,
                                              Predicate&& p) {
  return has_fixed<1>(state, std::forward<Predicate>(p));
}

template <typename InputIt, typename Predicate>
inline constexpr ParserState<InputIt> has_zero_or_one(
    ParserState<InputIt> state, Predicate&& p) {
  state = has_one(state, std::forward<Predicate>(p));
  state.status = Status::kOk;

  return state;
}

template <typename InputIt, typename Predicate>
inline constexpr ParserState<InputIt> has_zero_or_more(
    ParserState<InputIt> state, Predicate&& p) {
  while (true) {
    state = has_one(state, std::forward<Predicate>(p));

    if (!state.is_ok()) {
      state.status = Status::kOk;
      break;
    }
  }

  return state;
}

template <typename InputIt, typename Predicate>
ParserState<InputIt> operator|(ParserState<InputIt> state, Predicate&& pipe) {
  return std::invoke(std::forward<Predicate>(pipe), state);
}

template <typename InputIt>
inline constexpr auto is_digit_pipe =
    [](ParserState<InputIt> state) { return has_one(state, is_digit); };

template <typename InputIt>
inline constexpr auto is_zero_or_more_digits_pipe =
    [](ParserState<InputIt> state) {
      return has_zero_or_more(state, is_digit);
    };

template <typename InputIt>
inline constexpr auto is_dot_pipe =
    [](ParserState<InputIt> state) { return has_one(state, is_byte<'.'>); };

template <typename InputIt>
inline constexpr auto is_exponent_pipe =
    [](ParserState<InputIt> state) { return has_one(state, is_exponent); };

template <typename InputIt>
inline constexpr auto is_opt_minus_pipe = [](ParserState<InputIt> state) {
  return has_zero_or_one(state, is_byte<'-'>);
};

template <typename InputIt>
inline constexpr auto is_opt_plus_pipe = [](ParserState<InputIt> state) {
  return has_zero_or_one(state, is_byte<'+'>);
};

// template <typename InputIt>
// inline constexpr auto is_4_hex_pipe =
//     [](ParserState<InputIt> state) { return has_fixed<4>(state, is_hex); };

}  // namespace json
