#include <gtest/gtest.h>

#include <memory>
#include <string_view>
#include <vector>

#include "json/common/alloc.hpp"
#include "json/parser/json_parser.hpp"

using namespace json;
using namespace std;

struct TestCase {
  std::string_view input;
  JsonValue json_value;
  ErrorCode err;
};

TEST(JsonParser, ParseNumber) {
  vector<TestCase> test_cases{
      {"0", JsonValue{0}, ErrorCode::kOk},
      {"2147483647", JsonValue{2147483647}, ErrorCode::kOk},
      {"-2147483648", JsonValue{-2147483648}, ErrorCode::kOk},
      {"-9223372036854775808", JsonValue{-9223372036854775807LL - 1},
       ErrorCode::kOk},
      {"9223372036854775807", JsonValue{9223372036854775807LL}, ErrorCode::kOk},
      {"4.2", JsonValue{4.2}, ErrorCode::kOk},
      {"4.2e100", JsonValue{4.2e100}, ErrorCode::kOk},
      {"1e1200", JsonValue{HUGE_VALF}, ErrorCode::kOverflow},
  };

  MallocAllocator alloc;

  for (auto& ts : test_cases) {
    auto [json_value, err] =
        JsonParser::parse(ts.input.begin(), ts.input.end(), alloc);

    EXPECT_EQ(ts.err, err) << ts.input;
    EXPECT_EQ(ts.json_value, json_value) << ts.input;
  }
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}