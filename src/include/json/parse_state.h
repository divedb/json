#pragma once

#include <cstdint>
#include <string>

namespace json {

enum class Status : std::int8_t { kError, kEOF, kOk };

template <typename InputIt>
struct ParseState {
  using value_type = typename std::iterator_traits<InputIt>::value_type;

  InputIt first;
  InputIt last;
  InputIt ncursor;  // Mark for new position
  InputIt ocursor;  // Mark for old position

  int line_number;
  Status status;

  ParseState(InputIt first, InputIt last, Status status = Status::kOk)
      : first(first),
        last(last),
        ncursor(first),
        ocursor(ncursor),
        line_number(0),
        status(status) {}

  constexpr bool is_ok() const noexcept { return status == Status::kOk; }
  constexpr bool has_next() const noexcept { return ncursor != last; }
  value_type next() {
    ocursor = ncursor;
    ncursor++;

    return *ocursor;
  }
};

#define CHECK_PARSE_STATE(ps) \
  do {                        \
    if (!ps.is_ok()) {        \
      return ps;              \
    }                         \
  } while (0)

template <typename InputIt, typename Predicate>
auto operator|(ParseState<InputIt> ps, Predicate&& p)
    -> decltype(std::invoke(std::forward<Predicate>(p), ps)) {
  return std::invoke(std::forward<Predicate>(p), ps);
}

}  // namespace json
