#pragma once

#include <cerrno>
#include <climits>
#include <cstdlib>

#include "json/common/memory_context.hpp"
#include "json/parser/char_stream.hpp"



namespace json {

class NumberParser {
public:
  explicit NumberParser(MemoryContext mem_cxt) : mem_cxt_(mem_cxt) {}

  bool parse(CharStream& stream) {

}

private:
bool parse_int()

  MemoryContext& mem_cxt_;
};

}  // namespace json