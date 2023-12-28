#include "json/parser_state.h"

#include <gtest/gtest.h>

using namespace json;

TEST(FixedPipe, Ok) {
  Buffer buf("1234");
  ParserState state(buf.begin(), buf.end());

  state = state | is_4_hex_pipe;

  EXPECT_TRUE(state.is_ok());
  EXPECT_EQ(buf, state.buffer());
}

TEST(FixedPipe, Eof) {
  Buffer buf("123");
  ParserState state(buf.begin(), buf.end());

  state = state | is_4_hex_pipe;

  EXPECT_TRUE(state.status == Status::kEOF);
}

TEST(FixedPipe, Error) {
  Buffer buf("123x");
  ParserState state(buf.begin(), buf.end());

  state = state | is_4_hex_pipe;

  EXPECT_TRUE(state.status == Status::kError);
  EXPECT_EQ("123", state.buffer());
}

TEST(GreedyPipe, Ok) {
  Buffer buf("012012012");
  ParserState state(buf.begin(), buf.end());

  state = state | is_1_or_more_digits_pipe;

  EXPECT_TRUE(state.is_ok());
  EXPECT_EQ(buf, state.buffer());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}