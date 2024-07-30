#include "json/nodes/json_value.hpp"

#include <gtest/gtest.h>

using namespace std;
using namespace json;

TEST(JsonValue, Comparison) {
  JsonValue v1{1};
  JsonValue v2{4.2};
  JsonValue v3{4.2};

  JsonValue v4{1e3};
  JsonValue v5{1.2e4};
  JsonValue v6{1.2e4};

  EXPECT_FALSE(v1 == v2);
  EXPECT_EQ(v2, v3);

  EXPECT_FALSE(v4 == v5);
  EXPECT_EQ(v5, v6);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}