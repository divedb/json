#pragma once

#include <algorithm>
#include <deque>
#include <iostream>

#include "json/types.h"
#include "json/util.h"

namespace json {

enum class Status : u8 { kError, kEOF, kOk };
enum class PipeMode : u8 { kFixed, kGreedy };

template <typename InputIt>
struct ParserState {
 public:
  // Constructor.
  constexpr ParserState(InputIt first, InputIt last,
                        Status status = Status::kOk)
      : status(status), line_number(0), first(first), last(last) {}

  constexpr u8 get() const { return *first; }
  constexpr u8 next() { return *first++; }
  constexpr bool has_next() const noexcept { return first != last; }
  constexpr bool is_ok() const noexcept { return status == Status::kOk; }

  Status status;
  int line_number;

 private:
  InputIt first;
  InputIt last;
};

#define CHECK_PARSER_STATE(state) \
  do {                            \
    if (!state.is_ok()) {         \
      return state;               \
    }                             \
  } while (0)

template <typename Predicate>
class PipeBase {
 public:
  // Construct a pipe to address `n` elements with the specified predicate `fp`.
  explicit PipeBase(Predicate&& fp, int size)
      : PipeBase(std::move(fp), size, size) {}
  explicit PipeBase(const Predicate& fp, int size) : PipeBase(fp, size, size) {}
  explicit PipeBase(Predicate&& fp, int size, int cap)
      : buffer_(cap, 0), fp_(std::move(fp)), cursor_(0), size_(size) {}
  explicit PipeBase(const Predicate& fp, int size, int cap)
      : buffer_(cap, 0), fp_(fp), cursor_(0), size_(size) {}

  // Get the consumed bytes when data is piped.
  Buffer buffer() const noexcept { return buffer_.substr(0, cursor_); }

  // Get the last piped byte.
  u8 back() const noexcept {
    assert(cursor_ != 0);

    return buffer_[cursor_ - 1];
  }

  template <typename InputIt>
  constexpr ParserState<InputIt> apply(ParserState<InputIt> state) {
    CHECK_PARSER_STATE(state);

    for (cursor_ = 0; state.has_next() && cursor_ < size_;) {
      u8 b = state.next();
      buffer_[cursor_++] = b;

      if (!fp_(b)) {
        state.status = Status::kError;
        return state;
      }
    }

    if (cursor_ < size_) {
      state.status = Status::kEOF;
    }

    return state;
  }

 protected:
  Buffer buffer_;
  Predicate fp_;
  int cursor_;
  const int size_;
};

template <typename Predicate, PipeMode = PipeMode::kGreedy>
class Pipe : public PipeBase<Predicate> {
 public:
  using Base = PipeBase<Predicate>;

  constexpr static int kBufSize = 64;

  explicit Pipe(Predicate&& fp, int size)
      : Base(std::move(fp), size, kBufSize) {}
  explicit Pipe(const Predicate& fp, int size) : Base(fp, size, kBufSize) {}

  template <typename InputIt>
  constexpr ParserState<InputIt> operator()(ParserState<InputIt> state) {
    state = Base::template apply(state);
    CHECK_PARSER_STATE(state);

    while (state.has_next()) {
      u8 b = state.next();
      this->buffer_[this->cursor_++] = b;

      if (!this->fp_(b)) {
        state.status = Status::kError;
        break;
      }
    }

    return state;
  }
};

template <typename Predicate>
class Pipe<Predicate, PipeMode::kFixed> : public PipeBase<Predicate> {
 public:
  using Base = PipeBase<Predicate>;
  using PipeBase<Predicate>::PipeBase;

  template <typename InputIt>
  constexpr ParserState<InputIt> operator()(ParserState<InputIt> state) {
    return Base::template apply(state);
  }
};

template <typename Predicate, typename DP = std::decay_t<Predicate>>
auto make_fixed_pipe(Predicate&& p, int n) {
  return Pipe<DP, PipeMode::kFixed>{std::forward<DP>(p), n};
}

template <typename Predicate, typename DP = std::decay_t<Predicate>>
auto make_greedy_pipe(Predicate&& p, int n) {
  return Pipe<DP, PipeMode::kGreedy>{std::forward<DP>(p), n};
}

inline auto is_4_hex_pipe = make_fixed_pipe(is_hex, 4);
inline auto is_1_or_more_digits_pipe = make_greedy_pipe(is_digit, 1);

template <typename InputIt, typename Predicate, PipeMode mode>
ParserState<InputIt> operator|(ParserState<InputIt> state,
                               Pipe<Predicate, mode>& pipe) {
  return std::invoke(pipe, state);
}

}  // namespace json
