#include "json/nparser_state.h"

#include <gtest/gtest.h>

#include "json/util.h"

using namespace json;

TEST(Pipe, HasOne) {
  Pipe pipe(1, is_digit);
  EXPECT_EQ(PipeResult::kSuccess, pipe('1'));
}

TEST(Pipe, HasZeroOne) {
  PipeZeroOrOne pipe(is_digit);
  EXPECT_EQ(PipeResult::kSuccess, pipe('1'));

  pipe.reset();
  EXPECT_EQ(PipeResult::kNoConsumption, pipe('?'));
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}