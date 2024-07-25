#pragma once

#include <memory>
#include <string_view>

#include "json/common/memory_context.hpp"
#include "json/parser/json_parser.hpp"

namespace json {

class Document {
 public:
  Document() = default;

  std::shared_ptr<JsonNode> parse(std::string_view data) {
    auto stream = std::make_unique<MemoryCharStream>(data);
    Parser parser{mem_ctx_};
    JsonNode* jnode = parser.parse(*stream);
    std::shared_ptr<JsonNode> sp(jnode, DummyDeleter{});

    return sp;
  }

  void serialize() const;

 private:
  class DummyDeleter {
   public:
    void operator()(void*) {}
  };

  MemoryContext mem_ctx_;
};

}  // namespace json