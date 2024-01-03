#include "json/number_parser.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

using namespace json;

using StringIt = std::string::const_iterator;

struct TestCase {
  TestCase(const Buffer& input, bool exp_return, const Buffer& exp_buf)
      : input(input),
        state(input.begin(), input.end()),
        exp_return(exp_return),
        exp_buf(exp_buf) {}

  TestCase(const Buffer& input, const Buffer& exp_buf,
           const JsonValue& exp_value)
      : input(input),
        state(input.begin(), input.end()),
        exp_buf(exp_buf),
        exp_value(exp_value) {}

  Buffer input;
  ParserState<StringIt> state;
  bool exp_return;
  Buffer exp_buf;
  JsonValue exp_value;
};

TEST(NumberParser, ParseInt) {
  std::vector<TestCase> tests = {
      {"", false, ""},      {"   ", false, ""},     {"x", false, ""},
      {"?", false, ""},     {"00", false, "0"},     {"0", true, "0"},
      {"0    ", true, "0"}, {"1234", true, "1234"}, {"10xxx", true, "10"}};

  for (auto&& test : tests) {
    EXPECT_EQ(test.exp_return, parse_int(test.state))
        << "[" << test.input << "]";
    EXPECT_EQ(test.exp_buf, test.state.buffer()) << "[" << test.input << "]";
  }
}

TEST(NumberParser, ParseFrac) {
  std::vector<TestCase> tests = {{"1", false, ""},
                                 {".", false, "."},
                                 {".x", false, "."},
                                 {"..", false, "."},
                                 {".0", true, ".0"},
                                 {".00", true, ".00"},
                                 {".123456", true, ".123456"}};
  for (auto&& test : tests) {
    EXPECT_EQ(test.exp_return, parse_frac(test.state))
        << "[" << test.input << "]";
    EXPECT_EQ(test.exp_buf, test.state.buffer()) << "[" << test.input << "]";
  }
}

TEST(NumberParser, ParseExponent) {
  std::vector<TestCase> tests = {
      {"x", false, ""},      {"e", false, "e"},    {"E", false, "E"},
      {"e-", false, "e-"},   {"E-", false, "E-"},  {"e+", false, "e+"},
      {"E+", false, "E+"},   {"e0", true, "e0"},   {"E0", true, "E0"},
      {"e-0", true, "e-0"},  {"e+0", true, "e+0"}, {"e-0123", true, "e-0123"},
      {"e123", true, "e123"}};

  for (auto&& test : tests) {
    EXPECT_EQ(test.exp_return, parse_exponent(test.state));
    EXPECT_EQ(test.exp_buf, test.state.buffer()) << "[" << test.input << "]";
  }
}

TEST(NumberParser, ParseNumberAux) {
  bool has_frac;
  std::vector<TestCase> tests = {{"00", false, "0"},
                                 {"0.", false, "0."},
                                 {"0.a", false, "0."},
                                 {"0.0e", false, "0.0e"},
                                 {"0.0e-", false, "0.0e-"}};

  for (auto&& test : tests) {
    EXPECT_EQ(test.exp_return, parse_number_aux(test.state, has_frac))
        << "[" << test.input << "]";
    EXPECT_EQ(test.exp_buf, test.state.buffer()) << "[" << test.input << "]";
  }
}

// TODO(gc): need test underflow and overflow.
TEST(NumberParser, ParseNumber) {
  std::vector<TestCase> tests{
      {"0", "0", JsonValue{Number(0LL)}},
      {"0.1", "0.1", JsonValue{Number(0.1L)}},
      {"1e10", "1e10", JsonValue{Number(1e10L)}},
      {"-1.23", "-1.23", JsonValue{Number(-1.23L)}},
      {"-1.23E3", "-1.23E3", JsonValue{Number(-1.23e3L)}},
      {"-9.0e+3", "-9.0e+3", JsonValue{Number(-9.0e3L)}}};

  for (auto&& test : tests) {
    JsonValue json_value = parse_number(test.state);
    EXPECT_EQ(test.exp_buf, test.state.buffer()) << "[" << test.input << "]";
    EXPECT_TRUE(test.exp_value == json_value);
  }
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}