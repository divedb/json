#include "json/parser_state.h"

#include <gtest/gtest.h>

using namespace json;

TEST(FixedPipe, Ok) {
  Buffer buf("1234");
  ParserState state(buf.begin(), buf.end());

  state = state | is_4_hex_pipe;

  EXPECT_TRUE(state.is_ok());
  EXPECT_EQ(buf, is_4_hex_pipe.buffer());
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
  EXPECT_EQ('x', is_4_hex_pipe.back());
}

TEST(GreedyPipe, Ok) {
  Buffer buf("012012012");
  ParserState state(buf.begin(), buf.end());

  state = state | is_1_or_more_digits_pipe;

  EXPECT_TRUE(state.is_ok());
  EXPECT_EQ(buf, is_1_or_more_digits_pipe.buffer());
  EXPECT_EQ('2', is_1_or_more_digits_pipe.back());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}