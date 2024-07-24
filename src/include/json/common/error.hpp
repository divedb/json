#pragma once

#include <sstream>
#include <stdexcept>

namespace json {

template <typename... Args>
inline void __throw__(char const* filename, const size_t lineno,
                      Args&&... args) {
  std::ostringstream oss;

  oss << "[" << filename << "@" << lineno << "]: ";

  (oss << ... << std::forward<Args>(args));

  throw std::runtime_error(oss.str());
}

#define THROW_ERROR(...) __throw__(__FILE__, __LINE__, __VA_ARGS__)

}  // namespace json