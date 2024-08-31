#include "dump.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>

#include "json/dom/doc.hpp"

namespace json {

inline void Dumper::indent_with_spaces(int nspaces) {
  std::fill_n(std::ostream_iterator<char>(os_), nspaces, ' ');
}

inline void Dumper::visit_simple(JsonValue const& value, int current_indent) {
  assert(value.is_simple_type());

  if (value.is_null()) {
    os_ << value.as_null();
  } else if (value.is_bool()) {
    os_ << std::boolalpha << value.as_bool();
  } else if (value.is_number()) {
    os_ << value.as_number();
  } else if (value.is_string()) {
    os_ << value.as_string();
  } else {
    assert(0);
  }
}

inline void Dumper::visit_aggregate(JsonValue const& value,
                                    int current_indent) {
  assert(value.is_aggregate_type());

  if (value.is_object()) {
    auto const& object = *value.as_object();

    for (auto iter : object) {
      auto const& k = iter.first;
      auto const& v = iter.second;

      indent_with_spaces(current_indent);
      os_ << k << ": ";
      visit_json_value(v, current_indent);
      // ListSeparator.
      os_ << ",\n";
    }
  } else if (value.is_array()) {
    auto const& array = *value.as_array();

    for (auto const& v : array) {
      indent_with_spaces(current_indent);
      visit_json_value(v, current_indent);
      os_ << ",\n";
    }
  } else {
    assert(0);
  }
}

inline void Dumper::visit_json_value(JsonValue const& value,
                                     int current_indent) {
  if (value.is_simple_type()) {
    visit_simple(value, current_indent);
  } else {
    if (value.is_object()) {
      os_ << "{\n";
      visit_aggregate(value, current_indent + indent_);
      indent_with_spaces(current_indent);
      os_ << "}";
    } else if (value.is_array()) {
      os_ << "[\n";
      visit_aggregate(value, current_indent + indent_);
      indent_with_spaces(current_indent);
      os_ << "]";
    } else {
      assert(0);
    }
  }
}

void Dumper::visit_json_value(JsonValue const& value) {
  visit_json_value(value, 0);
}

}  // namespace json

int main(int argc, char** argv) {
  json::Document doc;
  doc.parse(R"({
    "hello": "world",
    "t": true ,
    "f": false,
    "n": null,
    "i": 123,
    "pi": 3.1416,
    "a": [1, 2, 3, 4]
  })");

  json::Dumper dumper(std::cout);
  dumper.visit_json_value(doc.root());

  return 0;
}