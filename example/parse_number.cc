#include <iostream>
#include <vector>

#include "json/number_parser.h"

using namespace std;
using namespace json;

int main(int argc, char** argv) {
  json::Buffer buf("0.1");
  json::ParserState state(buf.begin(), buf.end());
  auto v = json::parse_number(state);
  v.print();
}