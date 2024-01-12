#include "json/string_parser.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "json/pipe.h"

using namespace json;

using InputIt = typename std::string::const_iterator;

struct StringTest {
  std::string input;
  Buffer expect_buf;
  JsonValue json_value;
  Status expect_status{Status::kSucceed};
};

static void parse_string_test(const std::vector<StringTest>& tests) {
  for (auto&& test : tests) {
    auto& input = test.input;
    std::string suffix = ":[" + input + "]";
    ParserState<InputIt> state(input.begin(), input.end());
    JsonValue json_value = parse_string(state);

    EXPECT_EQ(test.expect_status, state.status) << suffix;
    EXPECT_EQ(test.expect_buf, state.buffer()) << suffix;

    if (state.is_ok()) {
      EXPECT_TRUE(test.json_value == json_value) << suffix;
    }
  }
}

TEST(StringParser, Succeed) {
  std::vector<StringTest> tests = {{"\"\"", "\"\"", JsonValue("\"\"")}};
  parse_string_test(tests);
}

// TEST(StringParser, ParseAscii) {
//   std::vector<StringTest> tests = {
//       {"\"\"", "\"\""},           {"\"a\"", "\"a\""},
//       {"\"aaa\"", "\"aaa\""},     {"\"1a2n\"", "\"1a2n\""},
//       {"\"AAAA\"", "\"AAAA\""},   {"\"1A2b3C4d\"", "\"1A2b3C4d\""},
//       {"\"44444\"", "\"44444\""}, {"\"0\"", "\"0\""},
//       {"\"ÿ\"", "\"ÿ\""},         {"\"\\n\"", "\"\n\""},
//       {"\"\\r\"", "\"\r\""},      {"\"\\f\"", "\"\f\""},
//       {"\"\\t\"", "\"\t\""},      {"\"\\b\"", "\"\b\""},
//       {"\"\\\\\"", "\"\\\""},     {"\"\\\"\\\"\"", "\"\"\"\""}};

//   internal_parse_string_test(tests);
// }

// TEST(StringParser, ParseUnicode) {
//   std::vector<StringTest> tests{
//       {"\"\\u1234\"", "\"\xe1\x88\xb4\""},
//       {"\"\\uaeae\"", "\"\xea\xba\xae\""},
//       {"\"\\uaE1F\"", "\"\xea\xb8\x9f\""},
//       {"\"\\ua1b1\"", "\"\xea\x86\xb1\""},
//       {"\"\\uA1B1\"", "\"\xea\x86\xb1\""},
//       {"\"\\uFFFF\"", "\"\xef\xbf\xbf\""},
//       {"\"\\u12341234\"", "\"\xe1\x88\xb4\x31\x32\x33\x34\""},
//       {"\"\\u1234 \"", "\"\xe1\x88\xb4\x20\""},
//       {"\"\\ud800\\udc00\"", "\"\xf0\x90\x80\x80\""},
//       {"\"\\ud800\\udc01\"", "\"\xf0\x90\x80\x81\""},
//       {"\"\\udb4A\\udf2A\"", "\"\xf3\xa2\xac\xaa\""},
//       {"\"\\udbff\\udfff\"", "\"\xf4\x8f\xbf\xbf\""},
//       {"\"\\udbff\\udfff1\"", "\"\xf4\x8f\xbf\xbf\x31\""}};

//   internal_parse_string_test(tests);
// }

// TEST(StringParser, ParseEOF) {
//   std::vector<StringTest> tests{
//       {"\"", {}, Status::kEOF},
//       {"\"\\\"", {}, Status::kEOF},
//       {"\"\\u\"", {}, Status::kEOF},
//       {"\"\\ud\"", {}, Status::kEOF},
//       {"\"\\ud80", {}, Status::kEOF},
//       {"\"\\ud800\\u\"", {}, Status::kEOF},
//       {"\"\\ud800\\u0\"", {}, Status::kEOF},
//       {"\"\\ud800\\u123", {}, Status::kEOF},
//   };

//   internal_parse_string_test(tests);
// }

// TEST(StringParser, ParseError) {
//   std::vector<StringTest> tests{
//       {"a", {}, Status::kError},
//       {"\"\\c\"", {}, Status::kError},
//       {"\"\\udc00\"", {}, Status::kError},
//       {"\"\\ud8001\"", {}, Status::kError},
//       {"\"\\ud800\\c\"", {}, Status::kError},
//       {"\"\\ud800\\u1234\"", {}, Status::kError},
//       {"\"\\ud800\\uxxxx\"", {}, Status::kError},
//       {"\"\\ud800\\u????\"", {}, Status::kError},
//       {"\"\\ud800\\udc0x\"", {}, Status::kError},
//   };

//   internal_parse_string_test(tests);
// }

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}