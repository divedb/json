#include <functional>
#include <iostream>

#include "json/string_parser.h"

using namespace std;

void parse_string(const std::string& s) {
  json::Buffer buf;
  json::ParseState ps(s.begin(), s.end());

  cout << json::internal_parse_string(ps, buf) << endl;
  cout << "[status]: " << static_cast<int>(ps.status) << endl;
  cout << "[buf]: " << buf << std::endl;
}

int main(int argc, char** argv) {
  //   parse_string("\"\\ud800\\udc00\"");
  //   parse_string("\"\\u\"");
  //   parse_string("\"\\ud80\"");
  parse_string("\"\u1234\"");

  function<void(int)> f([](int) {});
}