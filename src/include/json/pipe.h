#pragma once

#include <optional>

#include "json/utf.h"
#include "json/util.h"
#include "parser_state.h"

namespace json {

#define PRECHECK_STATE(state)   \
  do {                          \
    if (!state.is_ok()) return; \
  } while (0)

template <typename UnaryPredicate>
class PipeFixed {
 public:
  constexpr explicit PipeFixed(int sz, UnaryPredicate&& up)
      : fixed_(sz), up_(std::move(up)) {}

  constexpr explicit PipeFixed(int sz, const UnaryPredicate& up)
      : fixed_(sz), up_(up) {}

  template <typename InputIt>
  constexpr void operator()(ParserState<InputIt>& state) const {
    PRECHECK_STATE(state);

    int i;

    for (i = 0; i < fixed_ && state.has_next(); ++i) {
      u8 b = state.next();

      if (!this->up_(b)) {
        state.put(b);
        state.status = Status::kFailure;
        return;
      }

      state.buf_[state.cursor_++] = b;
    }

    if (i < fixed_) {
      state.status = Status::kEOF;
      return;
    }

    state.status = Status::kSucceed;
  }

 private:
  const int fixed_;
  UnaryPredicate up_;
};

template <typename Ret, typename... Args>
PipeFixed(int, Ret(Args...)) -> PipeFixed<Ret (*)(Args...)>;

template <typename UnaryPredicate>
class PipeOne {
 public:
  constexpr explicit PipeOne(UnaryPredicate&& up) : pipe_(1, std::move(up)) {}
  constexpr explicit PipeOne(const UnaryPredicate& up) : pipe_(1, up) {}

  template <typename InputIt>
  constexpr void operator()(ParserState<InputIt>& state) const {
    pipe_.operator()(state);
  }

 private:
  PipeFixed<UnaryPredicate> pipe_;
};

template <typename Ret, typename... Args>
PipeOne(Ret(Args...)) -> PipeOne<Ret (*)(Args...)>;

template <typename UnaryPredicate>
class PipeZeroOrOne {
 public:
  constexpr explicit PipeZeroOrOne(UnaryPredicate&& up)
      : pipe_one_(std::move(up)) {}
  constexpr explicit PipeZeroOrOne(const UnaryPredicate& up) : pipe_one_(up) {}

  template <typename InputIt>
  constexpr void operator()(ParserState<InputIt>& state) const {
    pipe_one_.operator()(state);
    state.status = Status::kSucceed;
  }

 private:
  PipeOne<UnaryPredicate> pipe_one_;
};

template <typename Ret, typename... Args>
PipeZeroOrOne(Ret(Args...)) -> PipeZeroOrOne<Ret (*)(Args...)>;

template <typename UnaryPredicate>
class PipeZeroOrMore {
 public:
  constexpr explicit PipeZeroOrMore(UnaryPredicate&& up)
      : pipe_one_(std::move(up)) {}
  constexpr explicit PipeZeroOrMore(const UnaryPredicate& up) : pipe_one_(up) {}

  template <typename InputIt>
  constexpr void operator()(ParserState<InputIt>& state) const {
    while (true) {
      if (pipe_one_.operator()(state); !state.is_ok()) {
        break;
      }
    }

    state.status = Status::kSucceed;
  }

 private:
  PipeOne<UnaryPredicate> pipe_one_;
};

template <typename Ret, typename... Args>
PipeZeroOrMore(Ret(Args...)) -> PipeZeroOrMore<Ret (*)(Args...)>;

class PipeEscape {
 public:
  static constexpr auto kSlashPipe = PipeOne(is_byte<'\\'>);
  static constexpr auto kHexPipe = PipeFixed(4, is_hex);
  static constexpr auto kUPipe = PipeOne(is_byte<'u'>);

  template <typename InputIt>
  constexpr void operator()(ParserState<InputIt>& state) const {
    PRECHECK_STATE(state);

    if (state | kSlashPipe; !state.is_ok()) {
      return;
    }

    parse_escape(state);
  }

 private:
  template <typename InputIt>
  constexpr bool parse_unicode(ParserState<InputIt>& state, i32& rune) const {
    if (state | kHexPipe; !state.is_ok()) {
      return false;
    }

    const int size = 4;
    auto first = state.buf_.begin() + state.cursor_ - size;
    auto last = first + size;
    rune = unicode_to_codepoint(first, last);

    return true;
  }

  template <typename InputIt>
  constexpr void parse_unicode(ParserState<InputIt>& state) const {
    i32 r1;
    i32 r2;
    auto old_cursor = state.cursor_;

    if (!parse_unicode(state, r1)) {
      return;
    }

    if (!is_surrogate(r1)) {
      auto encode = UTF8::encode(r1);
      std::copy(encode.begin(), encode.end(), old_cursor + state.buf_.begin());
      state.cursor_ = old_cursor + encode.size();
    } else if (is_high_surrogate(r1)) {
      old_cursor = state.cursor_;

      if (state | kSlashPipe | kUPipe; !state.is_ok()) {
        return;
      }

      if (!parse_unicode(state, r2)) {
        return;
      }

      if (is_low_surrogate(r2)) {
        auto encode = UTF8::encode(UTF16::decode(r1, r2));
        std::copy(encode.begin(), encode.end(),
                  old_cursor + state.buf_.begin());
        state.cursor_ = old_cursor + encode.size();
      } else {
        state.status = Status::kFailure;
      }
    } else {
      state.status = Status::kFailure;
    }
  }

  template <typename InputIt>
  constexpr void parse_escape(ParserState<InputIt>& state) const {
    auto b = state.next();

    switch (b) {
      case '"':
      case '\\':
      case '/':
        state.buf_[state.cursor_++] = b;
        break;
      case 'b':
        state.buf_[state.cursor_++] = '\b';
        break;
      case 'f':
        state.buf_[state.cursor_++] = '\f';
        break;
      case 'n':
        state.buf_[state.cursor_++] = '\n';
        break;
      case 'r':
        state.buf_[state.cursor_++] = '\r';
        break;
      case 't':
        state.buf_[state.cursor_++] = '\t';
        break;
      case 'u':
        parse_unicode(state);
        break;
      default:
        state.status = Status::kFailure;
        break;
    }
  }
};

template <typename InputIt, typename Pipe>
ParserState<InputIt>& operator|(ParserState<InputIt>& state, Pipe&& p) {
  return state.pipe(p);
}

inline constexpr auto digit_pipe = PipeOne(is_digit);
inline constexpr auto non_digit_pipe = PipeOne(is_non_digit);
inline constexpr auto zero_or_one_digit_pipe = PipeZeroOrOne(is_digit);
inline constexpr auto zero_or_more_digits_pipe = PipeZeroOrMore(is_digit);
inline constexpr auto escape_pipe = PipeEscape{};
inline constexpr auto sink_pipe = PipeOne(is_ascii);

}  // namespace json