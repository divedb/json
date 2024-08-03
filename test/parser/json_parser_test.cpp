#include "json/parser/json_parser.hpp"

#include <gtest/gtest.h>

#include "json/common/alloc.hpp"

using namespace json;
using namespace std;

struct TestCase {
  std::string_view input;
  JsonValue json_value;
  ErrorCode err{ErrorCode::kOk};
};

void internal_test(vector<TestCase> const& test_cases) {
  MallocAllocator alloc;

  for (auto& ts : test_cases) {
    auto [json_value, err] =
        JsonParser::parse(ts.input.begin(), ts.input.end(), alloc);

    EXPECT_EQ(ts.err, err) << ts.input;

    if (err == ErrorCode::kOk) {
      EXPECT_EQ(ts.json_value, json_value) << ts.input;
    }
  }
}

TEST(NumberParser, Basic) {
  vector<TestCase> test_cases{
      {"0", JsonValue{0}},
      {"2147483647", JsonValue{2147483647}},
      {"-2147483648", JsonValue{-2147483648}},
      {"-9223372036854775808", JsonValue{-9223372036854775807LL - 1}},
      {"9223372036854775807", JsonValue{9223372036854775807LL}},
      {"4.2", JsonValue{4.2}},
      {"4.2e100", JsonValue{4.2e100}},
      {"1e1200", JsonValue{HUGE_VALF}, ErrorCode::kOverflow},
  };

  internal_test(test_cases);
}

TEST(StringParser, Basic) {
  vector<TestCase> test_cases{
      {"\"\"", JsonValue{""}},
      {"\"a\"", JsonValue{"a"}},
      {"\"aaa\"", JsonValue{"aaa"}},
      {"\"1a2n\"", JsonValue{"1a2n"}},
      {"\"AAAA\"", JsonValue{"AAAA"}},
      {"\"1A2b3C4d\"", JsonValue{"1A2b3C4d"}},
      {"\"44444\"", JsonValue{"44444"}},
      {"\"0\"", JsonValue{"0"}},
      {"\"ÿ\"", JsonValue{"ÿ"}},
      {R"("\n")", JsonValue{"\n"}},
      {R"("\r")", JsonValue{"\r"}},
      {R"("\f")", JsonValue{"\f"}},
      {R"("\t")", JsonValue{"\t"}},
      {R"("\b")", JsonValue{"\b"}},
      {R"("\\")", JsonValue{"\\"}},
      {R"("\"\"")", JsonValue{"\"\""}},
  };

  internal_test(test_cases);
}

// TODO(gc): support surrogate pair.
TEST(StringParser, Unicode) {
  vector<TestCase> test_cases{
      {R"("\u1234")", JsonValue{"\xe1\x88\xb4"}},
      {R"("\uaeae")", JsonValue{"\xea\xba\xae"}},
      {R"("\uaE1F")", JsonValue{"\xea\xb8\x9f"}},
      {R"("\ua1b1")", JsonValue{"\xea\x86\xb1"}},
      {R"("\uA1B1")", JsonValue{"\xea\x86\xb1"}},
      {R"("\uFFFF")", JsonValue{"\xef\xbf\xbf"}},
      {R"("\u12341234")", JsonValue{"\xe1\x88\xb4\x31\x32\x33\x34"}},
      {R"("\u1234 ")", JsonValue{"\xe1\x88\xb4\x20"}},
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

TEST(ArrayParser, Basic) {
  MallocAllocator alloc;

  vector<TestCase> test_cases{
      {R"([])", JsonValueFactory::create_array(alloc)},
      {R"([1, 2, 3])", JsonValueFactory::create_array(alloc, 1, 2, 3)},
      {R"([ 1  ,   2    ,   3    ])",
       JsonValueFactory::create_array(alloc, 1, 2, 3)},
      {R"([ "1"  ,   "2"    ,   "3"    ])",
       JsonValueFactory::create_array(alloc, "1", "2", "3")},
      {R"([ "1"  ,   2    ,   "3"    ])",
       JsonValueFactory::create_array(alloc, "1", 2, "3")},
  };

  internal_test(test_cases);
}

TEST(ArrayParser, SubArray) {
  MallocAllocator alloc;

  vector<TestCase> test_cases{
      {"[[]]", JsonValueFactory::create_array(
                   alloc, JsonValueFactory::create_array(alloc))},
      {"[[1]]", JsonValueFactory::create_array(
                    alloc, JsonValueFactory::create_array(alloc, 1))},
      {"[[1], 2, [3, 4, 5]]",
       JsonValueFactory::create_array(
           alloc, JsonValueFactory::create_array(alloc, 1), 2,
           JsonValueFactory::create_array(alloc, 3, 4, 5))}};

  internal_test(test_cases);
}

TEST(ObjectParser, Basic) {
  MallocAllocator alloc;

  vector<TestCase> test_cases{
      {R"({"name": "jack", "age": 18})",
       JsonValueFactory::create_object(alloc, {"name", "age"},
                                       {JsonValue{"jack"}, JsonValue{18}})},
      {R"({"name"  :   "jack"    ,   "age" : 18, "hobbies": ["swimming", "running"], "salary": 12500, "is_male": true, "child": {"age": 6}})",
       JsonValueFactory::create_object(
           alloc, {"name", "age", "hobbies", "salary", "is_male", "child"},
           {JsonValueFactory::create_string("jack"),
            JsonValueFactory::create_number(18),
            JsonValueFactory::create_array(alloc, "swimming", "running"),
            JsonValueFactory::create_number(12500),
            JsonValueFactory::create_bool(true),
            JsonValueFactory::create_object(
                alloc, {"age"}, {JsonValueFactory::create_number(6)})})},
  };

  internal_test(test_cases);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}