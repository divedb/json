#include "json/dom/doc.hpp"

#include <gtest/gtest.h>

using namespace json;
using namespace std;

TEST(Dom, Basic) {
  Document doc;
  doc.parse(R"({
    "hello": "world",
    "t": true ,
    "f": false,
    "n": null,
    "i": 123,
    "pi": 3.1416,
    "a": [1, 2, 3, 4]
  })");

  ASSERT_TRUE(doc.is_object());

  EXPECT_TRUE(doc.has_member("hello"));
  EXPECT_TRUE(doc["hello"].is_string());

  EXPECT_TRUE(doc.has_member("age"));
  EXPECT_TRUE(doc.has_member("hobbies"));
  EXPECT_FALSE(doc.has_member("salary"));
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}