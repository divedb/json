#pragma once

#include <algorithm>

#include "json/parse_state.h"
#include "json/types.h"

namespace json {

template <JsonChar target>
inline constexpr auto is_byte = [](JsonChar input) { return input == target; };

inline constexpr bool is_hex(int c) {
  return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
         (c >= 'A' && c <= 'F');
}

template <std::size_t N, typename InputIter, typename Predicate>
inline constexpr ParseState<InputIter> has_fixed(ParseState<InputIter> ps,
                                                 Predicate&& p) {
  CHECK_PARSE_STATE(ps);

  if (std::distance(ps.ncursor, ps.last) < N) {
    ps.status = Status::kEOF;
  } else if (!std::all_of(ps.ncursor, ps.ncursor + N,
                          std::forward<Predicate>(p))) {
    ps.status = Status::kError;
  } else {
    ps.ocursor = ps.ncursor;
    ps.ncursor += N;
  }

  return ps;
}

template <typename InputIter, typename Predicate>
inline constexpr ParseState<InputIter> has_one(ParseState<InputIter> ps,
                                               Predicate&& p) {
  CHECK_PARSE_STATE(ps);

  return has_fixed<1>(ps, std::forward<Predicate>(p));
}

unsigned int next_power_of_2(unsigned int n) {
  if (n && !(n & (n - 1))) {
    return n;
  }

  unsigned int p = 1;
  while (p < n) {
    p <<= 1;
  }

  return p;
}

}  // namespace json