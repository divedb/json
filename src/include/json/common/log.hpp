#pragma once

#include <ostream>

namespace json {

template <typename... Args>
void json_log(std::ostream& os, char const* filename, const size_t lineno,
              Args&&... args) {
  os << filename << ':' << lineno << '\n';
  (os << ... << std::forward<Args>(args));

  os << std::endl;
}

#define LOG(os, ...) json_log(os, __FILE__, __LINE__, __VA_ARGS__)

}  // namespace json