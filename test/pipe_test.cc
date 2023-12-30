#include "json/pipe.h"

#include <gtest/gtest.h>

#include <sstream>

#include "json/parser_state.h"

using namespace json;

using StringIt = std::string::iterator;

#define AUTO_GEN_MACRO(str, pipes)                     \
  Buffer buf((str));                                   \
  ParserState<StringIt> state(buf.begin(), buf.end()); \
  state | pipes;

TEST(StringPipe, IsDigitOk) {
  AUTO_GEN_MACRO("1", is_digit_pipe)

  EXPECT_TRUE(state.is_ok());
  EXPECT_FALSE(state.has_next());
  EXPECT_EQ(buf, state.buffer());
  EXPECT_EQ(1, state.succeed_pipes);
}

TEST(StringPipe, IsDigitError) {
  AUTO_GEN_MACRO("x", is_digit_pipe)

  EXPECT_EQ(Status::kFailure, state.status);
  EXPECT_TRUE(state.has_next());
  EXPECT_TRUE(state.buffer().empty());
  EXPECT_EQ('x', state.next());
  EXPECT_FALSE(state.has_next());
  EXPECT_EQ(0, state.succeed_pipes);
}

TEST(StringPipe, IsDigitEOF) {
  AUTO_GEN_MACRO("", is_digit_pipe)

  EXPECT_EQ(Status::kEOF, state.status);
  EXPECT_FALSE(state.has_next());
  EXPECT_TRUE(state.buffer().empty());
  EXPECT_EQ(0, state.succeed_pipes);
}

TEST(StringPipe, IsZeroOrMoreDigits) {
  {
    AUTO_GEN_MACRO("", is_zero_or_more_digits_pipe)
    EXPECT_TRUE(state.is_ok());
    EXPECT_FALSE(state.has_next());
    EXPECT_TRUE(state.buffer().empty());
    EXPECT_EQ(1, state.succeed_pipes);
  }

  {
    AUTO_GEN_MACRO("01234567899876543210x", is_zero_or_more_digits_pipe)
    EXPECT_TRUE(state.is_ok());
    EXPECT_TRUE(state.has_next());
    EXPECT_EQ(Buffer("01234567899876543210"), state.buffer());
    EXPECT_EQ('x', state.next());
    EXPECT_FALSE(state.has_next());
    EXPECT_EQ(1, state.succeed_pipes);
  }
}

TEST(StringPipe, ChainedPipes) {
  {
    AUTO_GEN_MACRO("12", is_digit_pipe | is_digit_pipe)

    EXPECT_TRUE(state.is_ok());
    EXPECT_FALSE(state.has_next());
    EXPECT_EQ(buf, state.buffer());
    EXPECT_EQ(2, state.succeed_pipes);
  }

  {
    AUTO_GEN_MACRO("1x", is_digit_pipe | is_digit_pipe)

    EXPECT_EQ(Status::kFailure, state.status);
    EXPECT_TRUE(state.has_next());
    EXPECT_EQ(Buffer("1"), state.buffer());
    EXPECT_EQ(1, state.succeed_pipes);
  }

  {
    AUTO_GEN_MACRO("1", is_digit_pipe | is_digit_pipe)

    EXPECT_EQ(Status::kEOF, state.status);
    EXPECT_FALSE(state.has_next());
    EXPECT_EQ(Buffer("1"), state.buffer());
    EXPECT_EQ(1, state.succeed_pipes);
  }

  {
    AUTO_GEN_MACRO("1", is_digit_pipe | is_zero_or_more_digits_pipe)

    EXPECT_TRUE(state.is_ok());
    EXPECT_FALSE(state.has_next());
    EXPECT_EQ(Buffer("1"), state.buffer());
    EXPECT_EQ(2, state.succeed_pipes);
  }

  {
    AUTO_GEN_MACRO("123456789", is_digit_pipe | is_zero_or_more_digits_pipe)

    EXPECT_TRUE(state.is_ok());
    EXPECT_FALSE(state.has_next());
    EXPECT_EQ(Buffer("123456789"), state.buffer());
    EXPECT_EQ(2, state.succeed_pipes);
  }
}

TEST(StreamPipe, IsDigit) {
  using Iter = std::istream_iterator<char>;

  std::istringstream iss("12");
  Iter first(iss);
  Iter last;

  ParserState<Iter> state(first, last);

  state | is_digit_pipe;

  EXPECT_TRUE(state.is_ok());
  EXPECT_TRUE(state.has_next());
  EXPECT_EQ(Buffer("1"), state.buffer());
  EXPECT_EQ(1, state.succeed_pipes);
  EXPECT_EQ('2', state.next());
  EXPECT_FALSE(state.has_next());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}