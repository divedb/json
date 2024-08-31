#pragma once

#include <iosfwd>

namespace json {

class JsonValue;

class Dumper {
 public:
  explicit Dumper(std::ostream& os, int indent = 2)
      : os_(os), indent_(indent) {}
  void visit_json_value(JsonValue const& value);

 private:
  void indent_with_spaces(int nspaces);
  void visit_simple(JsonValue const& value, int current_indent);
  void visit_aggregate(JsonValue const& value, int current_indent);
  void visit_json_value(JsonValue const& value, int current_indent);

  std::ostream& os_;
  int indent_;
};

}  // namespace json