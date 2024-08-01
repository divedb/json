#pragma once

#include <sstream>
#include <stdexcept>

namespace json {

template <typename... Args>
void json_throw_error(char const* filename, const size_t lineno,
                      Args&&... args) {
  std::ostringstream oss;

  oss << "[" << filename << "@" << lineno << "]: ";
  (oss << ... << std::forward<Args>(args));

  throw std::runtime_error(oss.str());
}

#define THROW_ERROR(...) json_throw_error(__FILE__, __LINE__, __VA_ARGS__)

}  // namespace json