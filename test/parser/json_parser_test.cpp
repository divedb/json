#include <gtest/gtest.h>

#include "json/document/doc.hpp"

using namespace json;
using namespace std;

TEST(JsonParser, ParseNull) {
  Document doc;

  auto node = doc.parse("null");
  EXPECT_TRUE(node->is_null());
}

TEST(JsonParser, ParseBool) {
  Document doc;

  auto node = doc.parse("false");
  EXPECT_TRUE(node->is_bool());

  // node = doc.parse("true");
  // EXPECT_TRUE(node->is_bool());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}