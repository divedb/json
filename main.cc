#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <variant>

using namespace std;

int main(int argc, char** argv) {
  variant<int, long double> v1 = 1e10L;
  variant<int, long double> v2 = strtold("1e10", nullptr);

  float f1 = 1.2345691212;
  float f2 = 1.2345691212;

  cout << (f1 == f2) << endl;

  return 0;
}