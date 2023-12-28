#pragma once

#include <algorithm>
#include <deque>
#include <iostream>

#include "json/types.h"
#include "json/util.h"

namespace json {

enum class Status : u8 { kError, kEOF, kOk };
enum class PipeMode : u8 { kOptional, kFixed, kGreedy };

template <typename InputIt>
class ParserState {
 public:
  using value_type = typename std::iterator_traits<InputIt>::value_type;
  constexpr static const value_type kSentinel{};
  constexpr static const int kBufSize = 64;

  // Constructor.
  constexpr ParserState(InputIt first, InputIt last,
                        Status status = Status::kOk)
      : line_number(0),
        status(status),
        error_(kSentinel),
        cache_(kSentinel),
        first_(first),
        last_(last),
        cursor_(0),
        buffer_(kBufSize, 0) {}

  constexpr void put(u8 b) {
    assert(cache_ == kSentinel);
    cache_ = b;
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
  Buffer buffer() const noexcept { return buffer_.substr(0, cursor_); }

  int line_number;
  Status status;
  value_type error_;

 private:
  template <typename Predicate>
  friend class PipeBase;

  template <typename Predicate, PipeMode>
  friend class Pipe;

  value_type cache_;
  InputIt first_;
  InputIt last_;
  int cursor_;
  Buffer buffer_;
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

template <typename Predicate>
class PipeBase {
 public:
  // Construct a pipe to address `n` elements with the specified predicate `fp`.
  constexpr explicit PipeBase(Predicate&& fp, int size)
      : fp_(std::move(fp)), size_(size) {}
  constexpr explicit PipeBase(const Predicate& fp, int size)
      : fp_(fp), size_(size) {}

  template <typename InputIt>
  constexpr ParserState<InputIt>& apply(ParserState<InputIt>& state) {
    CHECK_PARSER_STATE(state);

    for (state.cursor_ = 0; state.has_next() && state.cursor_ < size_;) {
      u8 b = state.next();

      if (!fp_(b)) {
        state.put(b);
        state.status = Status::kError;
        return state;
      }

      state.buffer_[state.cursor_++] = b;
    }

    if (state.cursor_ < size_) {
      state.status = Status::kEOF;
    }

    return state;
  }

 protected:
  Predicate fp_;
  const int size_;
};

template <typename Predicate, PipeMode = PipeMode::kGreedy>
class Pipe : public PipeBase<Predicate> {
 public:
  using Base = PipeBase<Predicate>;
  using Base::Base;

  template <typename InputIt>
  constexpr ParserState<InputIt>& operator()(ParserState<InputIt>& state) {
    state = Base::template apply(state);
    CHECK_PARSER_STATE(state);

    while (state.has_next()) {
      u8 b = state.next();

      if (!this->fp_(b)) {
        state.put(b);
        state.status = Status::kError;
        break;
      }

      state.buffer_[state.cursor_++] = b;
    }

    return state;
  }
};

template <typename Predicate>
class Pipe<Predicate, PipeMode::kFixed> : public PipeBase<Predicate> {
 public:
  using Base = PipeBase<Predicate>;
  using Base::Base;

  template <typename InputIt>
  constexpr ParserState<InputIt> operator()(ParserState<InputIt> state) {
    return Base::template apply(state);
  }
};

template <typename Predicate>
class Pipe<Predicate, PipeMode::kOptional> : public PipeBase<Predicate> {
 public:
  using Base = PipeBase<Predicate>;
  using Base::Base;

  template <typename InputIt>
  constexpr ParserState<InputIt>& operator()(ParserState<InputIt>& state) {
    CHECK_PARSER_STATE(state);

    if (state.has_next()) {
      u8 b = state.next();

      if (this->fp_(b)) {
        state.buffer_[state.cursor_++] = b;
      } else {
        state.put(b);
      }
    }

    return state;
  }
};

template <typename Predicate, typename DP = std::decay_t<Predicate>>
constexpr auto make_fixed_pipe(Predicate&& p, int n) {
  return Pipe<DP, PipeMode::kFixed>{std::forward<DP>(p), n};
}

template <typename Predicate, typename DP = std::decay_t<Predicate>>
constexpr auto make_greedy_pipe(Predicate&& p, int n) {
  return Pipe<DP, PipeMode::kGreedy>{std::forward<DP>(p), n};
}

template <typename Predicate, typename DP = std::decay_t<Predicate>>
constexpr auto make_optional_pipe(Predicate&& p, int n) {
  return Pipe<DP, PipeMode::kOptional>{std::forward<DP>(p), n};
}

template <typename InputIt, typename Predicate, PipeMode mode>
ParserState<InputIt> operator|(ParserState<InputIt>& state,
                               Pipe<Predicate, mode>& pipe) {
  return std::invoke(pipe, state);
}

template <typename InputIt, typename Predicate, PipeMode mode>
ParserState<InputIt> operator|(ParserState<InputIt>& state,
                               Pipe<Predicate, mode>&& pipe) {
  return std::invoke(std::move(pipe), state);
}

inline auto is_4_hex_pipe = make_fixed_pipe(is_hex, 4);
inline auto is_1_or_more_digits_pipe = make_greedy_pipe(is_digit, 1);
inline auto is_0_or_more_digits_pipe = make_greedy_pipe(is_digit, 0);

}  // namespace json
