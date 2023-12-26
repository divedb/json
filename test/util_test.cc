#include "json/util.h"

#include <gtest/gtest.h>

#include <functional>
#include <iterator>
#include <string>
#include <vector>

template <typename InputIter>
struct TestCase {
  using reference = typename std::iterator_traits<InputIter>::reference;

  json::ParseState<InputIter> pstate;
  std::function<bool(reference)> pred;
  bool expected;
};

template <typename InputIter>
static void has_one_test(std::vector<TestCase<InputIter>>& tests) {
  for (auto&& test : tests) {
    EXPECT_EQ(test.expected, json::has_one(test.pstate, test.pred).is_ok());
  }
}

TEST(Util, HasOne) {
  std::vector<int> data{1, 2, 3, 4, 5};
  using InputIter = decltype(data.begin());

  std::vector<TestCase<InputIter>> tests = {
      {{data.begin(), data.begin()}, [](int v) { return v == 1; }, false},
      {{data.begin(), data.begin() + 1}, [](int v) { return v == 1; }, true},
      {{data.begin() + 3, data.end()}, [](int v) { return v >= 4; }, true}};

  has_one_test(tests);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}