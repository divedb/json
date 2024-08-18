#pragma once

#include <memory>
#include <string_view>

#include "json/common/memory_context.hpp"
#include "json/nodes/json_node.hpp"
#include "json/parser/json_parser.hpp"

namespace json {

class Document {
 public:
  Document() = default;

  JsonNode parse(std::string_view data) {}

  constexpr bool is_object() const;

  void serialize() const;

 private:
  JsonNode* root_{};
  MemoryContext mem_ctx_;
};

}  // namespace json