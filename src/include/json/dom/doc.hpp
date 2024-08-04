#pragma once

#include <memory>
#include <string_view>

#include "json/common/memory_context.hpp"
#include "json/parser/json_parser.hpp"

namespace json {

class Document {
 public:
  Document() = default;

  JsonNode parse(std::string_view data) {}

  void serialize() const;

 private:
  MemoryContext mem_ctx_;
};

}  // namespace json