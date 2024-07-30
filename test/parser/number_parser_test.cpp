#include "json/parser/number_parser.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <string_view>
#include <vector>

using namespace json;
using namespace std;

struct TestCase {
  ErrorCode err;
  std::string_view input;
  JsonValue json_value;
};

TEST(JsonParser, ParseNumber) {
  vector<TestCase> test_cases{
      {ErrorCode::kOk, "0", JsonValue{0}},
  };

  for (auto& ts : test_cases) {
    ErrorCode err;
    JsonValue json_value;

    parse_number(ts.input.begin(), ts.input.end(), json_value, err);
    EXPECT_EQ(ts.err, err);
    EXPECT_EQ(ts.json_value, json_value);
  }
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}