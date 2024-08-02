#include <gtest/gtest.h>

#include <vector>

#include "json/common/alloc.hpp"
#include "json/parser/json_parser.hpp"

using namespace json;
using namespace std;

struct TestCase {
  string input;
  string output;
  ErrorCode err{ErrorCode::kOk};
};

static void internal_test(vector<TestCase> const& test_cases) {
  MallocAllocator alloc;

  for (auto& ts : test_cases) {
    auto [json_value, err] =
        JsonParser::parse(ts.input.begin(), ts.input.end(), alloc);

    EXPECT_EQ(ts.err, err) << ts.input;

    if (err == ErrorCode::kOk) {
      EXPECT_EQ(JsonValue{ts.output}, json_value) << ts.input;
    }
  }
}

TEST(StringParser, Basic) {
  vector<TestCase> test_cases{
      {"\"\"", ""},           {"\"a\"", "a"},
      {"\"aaa\"", "aaa"},     {"\"1a2n\"", "1a2n"},
      {"\"AAAA\"", "AAAA"},   {"\"1A2b3C4d\"", "1A2b3C4d"},
      {"\"44444\"", "44444"}, {"\"0\"", "0"},
      {"\"ÿ\"", "ÿ"},         {R"("\n")", "\n"},
      {R"("\r")", "\r"},      {R"("\f")", "\f"},
      {R"("\t")", "\t"},      {R"("\b")", "\b"},
      {R"("\\")", "\\"},      {R"("\"\"")", "\"\""},
  };

  internal_test(test_cases);
}

// TODO(gc): support surrogate pair.
TEST(StringParser, Unicode) {
  vector<TestCase> test_cases{
      {R"("\u1234")", "\xe1\x88\xb4"},
      {R"("\uaeae")", "\xea\xba\xae"},
      {R"("\uaE1F")", "\xea\xb8\x9f"},
      {R"("\ua1b1")", "\xea\x86\xb1"},
      {R"("\uA1B1")", "\xea\x86\xb1"},
      {R"("\uFFFF")", "\xef\xbf\xbf"},
      {R"("\u12341234")", "\xe1\x88\xb4\x31\x32\x33\x34"},
      {R"("\u1234 ")", "\xe1\x88\xb4\x20"},
      //   {"\"\\ud800\\udc00\"", "\"\xf0\x90\x80\x80\""},
      //   {"\"\\ud800\\udc01\"", "\"\xf0\x90\x80\x81\""},
      //   {"\"\\udb4A\\udf2A\"", "\"\xf3\xa2\xac\xaa\""},
      //   {"\"\\udbff\\udfff\"", "\"\xf4\x8f\xbf\xbf\""},
      //   {"\"\\udbff\\udfff1\"", "\"\xf4\x8f\xbf\xbf\x31\""},
  };

  internal_test(test_cases);
}

TEST(StringParser, EndOfFile) {
  vector<TestCase> test_cases{
      {"\"", {}, ErrorCode::kEOF},
      {R"("\")", {}, ErrorCode::kEOF},
      {R"("\u")", {}, ErrorCode::kEOF},
      {R"("\ud")", {}, ErrorCode::kEOF},
      //   {"\"\\ud800\\u\"", {}, ErrorCode::kEOF},
      //   {"\"\\ud800\\u0\"", {}, ErrorCode::kEOF},
      //   {"\"\\ud800\\u123", {}, ErrorCode::kEOF},
  };

  internal_test(test_cases);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}