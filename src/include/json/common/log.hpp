#pragma once

namespace json {

#define LOG(os, ...) \
  os << '[' << __func__ << '@' << __LINE__ << ']' << __VA_ARGS__

}  // namespace json