#include "json/number.hpp"

#include <gtest/gtest.h>

using namespace json;

TEST(Number, Comparison) {
  Number n1{1};
  Number n2{2.0};

  EXPECT_TRUE(n1 != n2);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}