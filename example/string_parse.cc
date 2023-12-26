#include <iostream>

#include "json/string_parser.h"

using namespace std;

int main(int argc, char** argv) {
  string s = "\"\\ud800\\udc00\"";
  json::ParseState ps(s.begin(), s.end());
  json::Buffer buf;

  json::internal_parse_string(ps, buf);

  cout << "buf = " << buf << endl;
}