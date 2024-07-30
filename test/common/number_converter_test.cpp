#include "json/common/number_converter.hpp"

#include <gtest/gtest.h>

#include <string>
#include <vector>

using namespace json;
using namespace std;

struct ErrnoTransaction {
  ~ErrnoTransaction() { errno = old_errno; }

  int old_errno{errno};
};

template <typename T>
struct TestCaseBase {
  std::string input;
  T output;
  NumberConverter::State state;
};

TEST(NumberConverter, Long) {
  using TestCase = TestCaseBase<long>;

  ErrnoTransaction trans;

  vector<TestCase> test_cases{
      {"0", 0, NumberConverter::State::kOk},
      {"-2147483648", -2147483648, NumberConverter::State::kOk},
      {"2147483647", 2147483647, NumberConverter::State::kOk},
      {"0.12", 0, NumberConverter::State::kOk},
      {"9223372036854775807", 9223372036854775807, NumberConverter::State::kOk},
      {"9223372036854775808", 9223372036854775807,
       NumberConverter::State::kOverflow},
  };

  for (auto& ts : test_cases) {
    NumberConverter conv;
    long v = conv.operator()<long>(ts.input.c_str(), nullptr);

    EXPECT_EQ(ts.output, v);
    EXPECT_EQ(ts.state, conv.state());
  }
}

TEST(NumberConverter, Double) {
  using TestCase = TestCaseBase<double>;

  ErrnoTransaction trans;

  vector<TestCase> test_cases{
      {"4.2", 4.2, NumberConverter::State::kOk},
      {"0.0", 0, NumberConverter::State::kOk},
      {"1e10", 1e10, NumberConverter::State::kOk},
      {"-1.2e3", -1.2e3, NumberConverter::State::kOk},
      {"1e1200", HUGE_VALF, NumberConverter::State::kOverflow},
  };

  for (auto const& ts : test_cases) {
    NumberConverter conv;
    auto v = conv.operator()<double>(ts.input.c_str(), nullptr);

    EXPECT_EQ(ts.output, v) << ts.input;
    EXPECT_EQ(ts.state, conv.state()) << ts.input;
  }
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}