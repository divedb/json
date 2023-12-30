#pragma once

#include "json/error.h"
#include "json/types.h"

namespace json {

template <typename UnaryPredicate>
class Pipe;

enum class Status : u8 { kFailure = 0, kSucceed = 1, kEOF = 2 };

template <typename InputIt>
class ParserState {
 public:
  constexpr static const u8 kSentinel{};
  // TODO(gc): do we need to utilize the SSO?
  constexpr static const int kBufSize = 256;

  // Constructor.
  constexpr ParserState(InputIt first, InputIt last,
                        Status status = Status::kSucceed)
      : cache(kSentinel),
        status(status),
        line_number(0),
        succeed_pipes(0),
        cursor_(0),
        buf_(kBufSize, 0),
        first_(first),
        last_(last) {}

  // Put a byte back to the cache. The cache can only store one byte at a time.
  // This limitation arises because the input iterator can only traverse the
  // data in a single pass.
  constexpr void put(u8 b) {
    assert(cache == kSentinel);

    cache = b;
  }

  constexpr u8 back() const {
    assert(cursor_ != 0);

    return buf_[cursor_ - 1];
  }

  constexpr u8 pop_back() {
    assert(cursor_ != 0);
    cursor_--;

    return buf_[cursor_];
  }

  constexpr u8 next() {
    if (cache != kSentinel) {
      auto res = cache;
      cache = kSentinel;

      return res;
    }

    return *first_++;
  }

  constexpr bool has_next() const noexcept {
    return cache != kSentinel || first_ != last_;
  }

  constexpr bool is_ok() const noexcept { return status == Status::kSucceed; }

  // Get the consumed bytes when data is piped.
  Buffer buffer() const noexcept { return buf_.substr(0, cursor_); }

  template <typename UnaryPredicate>
  ParserState& pipe(UnaryPredicate&& up) {
    if (!is_ok()) {
      return *this;
    }

    status = std::forward<UnaryPredicate>(up)(*this);

    if (status == Status::kSucceed) {
      succeed_pipes++;
    }

    return *this;
  }

  u8 cache;
  Status status;
  int line_number;
  int succeed_pipes;
  Error error;

 private:
  template <typename UnaryPredicate>
  friend class Pipe;

  int cursor_;
  Buffer buf_;
  InputIt first_;
  InputIt last_;
};

}  // namespace json