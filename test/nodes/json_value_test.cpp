#include <gtest/gtest.h>

#include "json/common/alloc.hpp"
#include "json/parser/json_parser.hpp"

using namespace std;
using namespace json;

TEST(JsonValue, Number) {
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

TEST(JsonValue, Array) {
  MallocAllocator alloc;

  // Create an empty array.
  auto v1 = JsonValueFactory::create_default_array(alloc);
  auto v2 = JsonValueFactory::create_default_array(alloc);
  auto v3 = JsonValueFactory::create_default_array(alloc);

  for (int i = 0; i < 10; i++) {
    v1.as_array()->append(JsonValue{i});
    v2.as_array()->append(JsonValue{i});

    if (i % 2 == 0) {
      v3.as_array()->append(JsonValue{i});
    }
  }

  EXPECT_EQ(v1, v2);
  EXPECT_NE(v2, v3);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}