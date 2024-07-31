#include "json/unicode/utf8.hpp"

#include <gtest/gtest.h>

#include <string>
#include <vector>

using namespace std;
using namespace json;

struct TestCase {
  int32_t codepoint;
  string expected;
};

static void internal_test(vector<TestCase> const& test_cases) {
  char buf[4];

  for (auto& ts : test_cases) {
    int n = UTF8::encode(buf, ts.codepoint);

    EXPECT_EQ(ts.expected, string(buf, n));
  }
}

// TODO(gc): how to test on 0x00?
TEST(UTF8, Encode1Byte) {
  vector<TestCase> test_cases{
      {0x01, "\x01"},
      {0x61, "a"},
      {0x7f, "\x7F"},
  };

  internal_test(test_cases);
}

TEST(UTF8, Encode2Bytes) {
  vector<TestCase> test_cases{
      {0x80, "\xc2\x80"},
      {0xff, "\xc3\xbf"},
      {0x7ff, "\xdf\xbf"},
  };

  internal_test(test_cases);
}

TEST(UTF8, Encode3Bytes) {
  vector<TestCase> test_cases{
      {0x800, "\xe0\xa0\x80"},
      {0xbbbb, "\xeb\xae\xbb"},
      {0xffff, "\xef\xbf\xbf"},
  };

  internal_test(test_cases);
}

TEST(UTF8, Encode4Bytes) {
  vector<TestCase> test_cases{
      {0x10000, "\xf0\x90\x80\x80"},
      {0x1abcd, "\xf0\x9a\xaf\x8d"},
      {0x10ffff, "\xf4\x8f\xbf\xbf"},
  };

  internal_test(test_cases);
}

// TEST(UTF8, encode_invalid) {
//   std::vector<TestCase> tests{
//       {0xd800}, {0xdbff}, {0xdc00}, {0xdfff}, {0x110000}};

//   for (auto&& test : tests) {
//     EXPECT_THROW(json::UTF8::encode(test.codepoint), std::invalid_argument);
//   }
// }

// TEST(UTF16Decode, BMP) {
//   auto r = json::kSurrSelf;

//   for (auto r1 = json::kSurr1; r1 < json::kSurr2; r1++) {
//     for (auto r2 = json::kSurr2; r2 < json::kSurr3; r2++, r++) {
//       EXPECT_EQ(r, json::UTF16::decode(r1, r2));
//     }
//   }
// }

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}