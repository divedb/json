#include "json/string_parser.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

using namespace json;

using InputIt = typename std::string::const_iterator;

struct StringTest {
  std::string input;
  Buffer expect_buf;
  Status expect_status{Status::kOk};
};

// TEST(StringParser, ParseBMP) {
//   std::vector<UnicodeTest> tests{{"", 0, Status::kEOF},
//                                  {"\\", 0, Status::kEOF},
//                                  {"\\u", 0, Status::kEOF},
//                                  {"\\u1", 0, Status::kEOF},
//                                  {"\\uz", 0, Status::kEOF},
//                                  {"\\u123", 0, Status::kEOF},
//                                  {"abc", 0, Status::kError},
//                                  {"\\a", 0, Status::kError},
//                                  {"\\u123g", 0, Status::kError},
//                                  {"\\u 1234", 0, Status::kError},
//                                  {"\\u0000", 0, Status::kOk},
//                                  {"\\u1234", 0x1234, Status::kOk},
//                                  {"\\uaeae", 0xaeae, Status::kOk},
//                                  {"\\uaE1F", 0xae1f, Status::kOk},
//                                  {"\\ua1b1", 0xa1b1, Status::kOk},
//                                  {"\\uA1B1", 0xa1b1, Status::kOk},
//                                  {"\\uFFFF", 0xffff, Status::kOk},
//                                  {"\\u12341234", 0x1234, Status::kOk},
//                                  {"\\u1234 ", 0x1234, Status::kOk}};

//   parse_unicode_test(tests, parse_bmp<InputIt>);
// }

// TEST(StringParser, ParseSurrogate) {
//   std::vector<UnicodeTest> tests{
//       {"", 0, Status::kEOF},
//       {"\\", 0, Status::kEOF},
//       {"\\u", 0, Status::kEOF},
//       {"\\ud", 0, Status::kEOF},
//       {"\\ud80", 0, Status::kEOF},
//       {"\\ud800", 0, Status::kEOF},
//       {"\\ud800\\", 0, Status::kEOF},
//       {"\\ud800\\u", 0, Status::kEOF},
//       {"\\ud800\\u0", 0, Status::kEOF},
//       {"\\ud800\\u123", 0, Status::kEOF},
//       {"a", 0, Status::kError},
//       {"\\c", 0, Status::kError},
//       {"\\u1234", 0, Status::kError},
//       {"\\u8a8a", 0, Status::kError},
//       {"\\ud7ff", 0, Status::kError},
//       {"\\udc00", 0, Status::kError},
//       {"\\ud8001", 0, Status::kError},
//       {"\\ud800\\c", 0, Status::kError},
//       {"\\ud800\\u1234", 0, Status::kError},
//       {"\\ud800\\uxxxx", 0, Status::kError},
//       {"\\ud800\\u????", 0, Status::kError},
//       {"\\ud800\\udc0x", 0, Status::kError},
//       {"\\ud800\\udc00", 0x10000, Status::kOk},
//       {"\\ud800\\udc01", 0x10001, Status::kOk},
//       {"\\udb4A\\udf2A", 0xe2b2a, Status::kOk},
//       {"\\udbff\\udfff", 0x10ffff, Status::kOk},
//       {"\\udbff\\udfff1", 0x10ffff, Status::kOk},
//   };

//   parse_unicode_test(tests, parse_surrogate<InputIt>);
// }

static void internal_string_test(const std::vector<StringTest>& tests) {
  for (auto&& test : tests) {
    Buffer buf;
    auto& input = test.input;
    ParseState<decltype(input.begin())> ps(input.begin(), input.end());
    internal_parse_string(ps, buf);

    EXPECT_EQ(test.expect_status, ps.status) << ":[" << input << "]";

    if (ps.is_ok()) {
      EXPECT_EQ(test.expect_buf, buf) << ":[" << input << "]";
    }
  }
}

TEST(StringParser, ParseAscii) {
  std::vector<StringTest> tests = {
      {"\"\"", "\"\""},           {"\"a\"", "\"a\""},
      {"\"aaa\"", "\"aaa\""},     {"\"1a2n\"", "\"1a2n\""},
      {"\"AAAA\"", "\"AAAA\""},   {"\"1A2b3C4d\"", "\"1A2b3C4d\""},
      {"\"44444\"", "\"44444\""}, {"\"0\"", "\"0\""},
      {"\"ÿ\"", "\"ÿ\""},         {"\"\\n\"", "\"\n\""},
      {"\"\\r\"", "\"\r\""},      {"\"\\f\"", "\"\f\""},
      {"\"\\t\"", "\"\t\""},      {"\"\\b\"", "\"\b\""},
      {"\"\\\\\"", "\"\\\""},     {"\"\\\"\\\"\"", "\"\"\"\""}};

  internal_string_test(tests);
}

TEST(StringParser, ParseUnicode) {
  std::vector<StringTest> tests{

      //   {"a", 0, Status::kError},
      //   {"\\c", 0, Status::kError},
      //   {"\\u1234", 0, Status::kError},
      //   {"\\u8a8a", 0, Status::kError},
      //   {"\\ud7ff", 0, Status::kError},
      //   {"\\udc00", 0, Status::kError},
      //   {"\\ud8001", 0, Status::kError},
      //   {"\\ud800\\c", 0, Status::kError},
      //   {"\\ud800\\u1234", 0, Status::kError},
      //   {"\\ud800\\uxxxx", 0, Status::kError},
      //   {"\\ud800\\u????", 0, Status::kError},
      //   {"\\ud800\\udc0x", 0, Status::kError},
      {"\"\\u1234\"", "\"\xe1\x88\xb4\""},
      {"\"\\uaeae\"", "\"\xea\xba\xae\""},
      {"\"\\uaE1F\"", "\"\xea\xb8\x9f\""},
      {"\"\\ua1b1\"", "\"\xea\x86\xb1\""},
      {"\"\\uA1B1\"", "\"\xea\x86\xb1\""},
      {"\"\\uFFFF\"", "\"\xef\xbf\xbf\""},
      {"\"\\u12341234\"", "\"\xe1\x88\xb4\x31\x32\x33\x34\""},
      {"\"\\u1234 \"", "\"\xe1\x88\xb4\x20\""},
      {"\"\\ud800\\udc00\"", "\"\xf0\x90\x80\x80\""},
      {"\"\\ud800\\udc01\"", "\"\xf0\x90\x80\x81\""},
      {"\"\\udb4A\\udf2A\"", "\"\xf3\xa2\xac\xaa\""},
      {"\"\\udbff\\udfff\"", "\"\xf4\x8f\xbf\xbf\""},
      {"\"\\udbff\\udfff1\"", "\"\xf4\x8f\xbf\xbf\x31\""}};

  internal_string_test(tests);
}

TEST(StringParser, ParseEOF) {
  std::vector<StringTest> tests{
      {"\"", {}, Status::kEOF},
      {"\"\\\"", {}, Status::kEOF},
      {"\"\\u\"", {}, Status::kEOF},
      {"\"\\ud\"", {}, Status::kEOF},
      {"\"\\ud80", {}, Status::kEOF},
      {"\"\\ud800\\u\"", {}, Status::kEOF},
      {"\"\\ud800\\u0\"", {}, Status::kEOF},
      {"\"\\ud800\\u123", {}, Status::kEOF},
  };

  internal_string_test(tests);
}

TEST(StringParser, ParseError) {
  std::vector<StringTest> tests{{"\"\\ud800\"", {}, Status::kError},
                                {"\"\\ud800\\\"", {}, Status::kError}};
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}