#include "json/utf.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

struct TestCase {
  json::i32 codepoint;
  json::Buffer expected;
};

static void test_encode(const std::vector<TestCase>& tests) {
  for (auto&& test : tests) {
    EXPECT_EQ(test.expected, json::UTF8::encode(test.codepoint));
  }
}

// TODO(gc): how to test on 0x00?
TEST(UTF8, encode_1_byte) {
  std::vector<TestCase> tests{{0x01, "\x01"}, {0x61, "a"}, {0x7f, "\x7F"}};
  test_encode(tests);
}

TEST(UTF8, encode_2_bytes) {
  std::vector<TestCase> tests{{0x80, "\xc2\x80"}, {0xff, "\xc3\xbf"}, {0x7ff, "\xdf\xbf"}};
  test_encode(tests);
}

TEST(UTF8, encode_3_bytes) {
  std::vector<TestCase> tests{{0x800, "\xe0\xa0\x80"}, {0xbbbb, "\xeb\xae\xbb"}, {0xffff, "\xef\xbf\xbf"}};
  test_encode(tests);
}

TEST(UTF8, encode_4_bytes) {
  std::vector<TestCase> tests{
      {0x10000, "\xf0\x90\x80\x80"}, {0x1abcd, "\xf0\x9a\xaf\x8d"}, {0x10ffff, "\xf4\x8f\xbf\xbf"}};

  test_encode(tests);
}

TEST(UTF8, encode_invalid) {
  std::vector<TestCase> tests{{0xd800}, {0xdbff}, {0xdc00}, {0xdfff}, {0x110000}};

  for (auto&& test : tests) {
    EXPECT_THROW(json::UTF8::encode(test.codepoint), std::invalid_argument);
  }
}

TEST(UTF16Decode, BMP) {
  auto r = json::kSurrSelf;

  for (auto r1 = json::kSurr1; r1 < json::kSurr2; r1++) {
    for (auto r2 = json::kSurr2; r2 < json::kSurr3; r2++, r++) {
      EXPECT_EQ(r, json::UTF16::decode(r1, r2));
    }
  }
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}