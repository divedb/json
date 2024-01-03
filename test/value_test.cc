#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "json/json_value.h"

TEST(Value, Number) {
  json::JsonValue json_value = json::Number{12LL};
  EXPECT_EQ(json::JsonType::kNumber, json_value.type());
  EXPECT_EQ(json_value, json::Number{12LL});
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}