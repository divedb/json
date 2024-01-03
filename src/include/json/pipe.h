#pragma once

#include <optional>

#include "json/util.h"
#include "parser_state.h"

namespace json {

template <typename UnaryPredicate>
class Pipe {
 public:
  constexpr explicit Pipe(int sz, UnaryPredicate&& up)
      : fixed_(sz), up_(std::move(up)) {}

  constexpr explicit Pipe(int sz, const UnaryPredicate& up)
      : fixed_(sz), up_(up) {}

  template <typename InputIt>
  constexpr Status operator()(ParserState<InputIt>& state) const {
    int i;

    for (i = 0; i < fixed_ && state.has_next(); ++i) {
      u8 b = state.next();

      if (!this->up_(b)) {
        state.put(b);

        return Status::kFailure;
      }

      state.buf_[state.cursor_++] = b;
    }

    if (i < fixed_) {
      return Status::kEOF;
    }

    return Status::kSucceed;
  }

 private:
  const int fixed_;
  UnaryPredicate up_;
};

template <typename UnaryPredicate>
class PipeOne {
 public:
  constexpr explicit PipeOne(UnaryPredicate&& up) : pipe_(1, std::move(up)) {}
  constexpr explicit PipeOne(const UnaryPredicate& up) : pipe_(1, up) {}

  template <typename InputIt>
  constexpr Status operator()(ParserState<InputIt>& state) const {
    return pipe_.operator()(state);
  }

 private:
  Pipe<UnaryPredicate> pipe_;
};

template <typename UnaryPredicate>
class PipeZeroOrOne {
 public:
  constexpr explicit PipeZeroOrOne(UnaryPredicate&& up)
      : pipe_one_(std::move(up)) {}
  constexpr explicit PipeZeroOrOne(const UnaryPredicate& up) : pipe_one_(up) {}

  template <typename InputIt>
  constexpr Status operator()(ParserState<InputIt>& state) const {
    pipe_one_.operator()(state);

    return Status::kSucceed;
  }

 private:
  PipeOne<UnaryPredicate> pipe_one_;
};

template <typename UnaryPredicate>
class PipeZeroOrMore {
 public:
  constexpr explicit PipeZeroOrMore(UnaryPredicate&& up)
      : pipe_one_(std::move(up)) {}
  constexpr explicit PipeZeroOrMore(const UnaryPredicate& up) : pipe_one_(up) {}

  template <typename InputIt>
  constexpr Status operator()(ParserState<InputIt>& state) const {
    while (pipe_one_.operator()(state) == Status::kSucceed) {
    }

    return Status::kSucceed;
  }

 private:
  PipeOne<UnaryPredicate> pipe_one_;
};

class EscapePipe {
 public:
  static constexpr std::optional<u8> escape_char_to_original(u8 escaped_char) {
    switch (escaped_char) {
      case '"':
      case '\\':
      case '/':
        return escaped_char;
      case 'b':
        return '\b';
      case 'f':
        return '\f';
      case 'n':
        return '\n';
      case 'r':
        return '\r';
      case 't':
        return '\t';
      default:
        return std::nullopt;
    }
  }

  template <typename InputIt>
  constexpr Status operator()(ParserState<InputIt>& state) const {
    IF_EOF_RETURN(state, Status::kEOF);

    if (auto b = state.next(); b != '\\') {
      return Status::kFailure;
    }

    IF_EOF_RETURN(state, Status::kEOF);

    std::optional<u8> opt;

    if (opt = error_type_to_string(state.next()); !opt.has_value()) {
      return Status::kFailure;
    }

    state.buf_[state.cursor_++] = opt.value();

    return Status::kSucceed;
  }
};

template <typename Ret, typename... Args>
Pipe(int, Ret(Args...)) -> Pipe<Ret (*)(Args...)>;

template <typename Ret, typename... Args>
PipeOne(Ret(Args...)) -> PipeOne<Ret (*)(Args...)>;

template <typename Ret, typename... Args>
PipeZeroOrOne(Ret(Args...)) -> PipeZeroOrOne<Ret (*)(Args...)>;

template <typename Ret, typename... Args>
PipeZeroOrMore(Ret(Args...)) -> PipeZeroOrMore<Ret (*)(Args...)>;

template <typename InputIt, typename Pipe>
ParserState<InputIt>& operator|(ParserState<InputIt>& state, Pipe&& p) {
  return state.pipe(p);
}

inline constexpr auto is_digit_pipe = PipeOne(is_digit);
inline constexpr auto is_non_digit_pipe = PipeOne(is_non_digit);
inline constexpr auto is_zero_or_one_digit_pipe = PipeZeroOrOne(is_digit);
inline constexpr auto is_zero_or_more_digits_pipe = PipeZeroOrMore(is_digit);
inline constexpr auto is_escape_pipe = EscapePipe{};

}  // namespace json