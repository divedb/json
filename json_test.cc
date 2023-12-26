#include <gtest/gtest.h>

#include <iostream>
#include <vector>

#include "json_parse.h"

using namespace std;
using namespace json;

struct NumTestCase {
  const char* input;
  Number expect;
};

TEST(Number, Integer) {
  JsonValue json_value;
  vector<NumTestCase> test_cases = {{"42", Number{42LL}}, {"1e0", Number{1LL}}, {"1e3", Number{1000LL}}};

  for (auto&& ts : test_cases) {
    ParseState ps = json::parse_number(ts.input, json_value);
    EXPECT_TRUE(ps.is_ok());

    Number num = json_value.get<Number>();
    cout << num.get<BigInteger>() << endl;
    EXPECT_TRUE(ts.expect == num);
  }
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}