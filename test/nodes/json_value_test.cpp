#include "json/nodes/json_value.hpp"

#include <gtest/gtest.h>

using namespace std;
using namespace json;

TEST(JsonValue, Comparison) {
  JsonValue v1{1};
  JsonValue v2{2.0};

  EXPECT_FALSE(v1 == v2);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}