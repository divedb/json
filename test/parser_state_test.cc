#include "json/parser_state.h"

#include <gtest/gtest.h>

using namespace json;

using InputIt = std::string::iterator;

class StringPipe : public ::testing::Test {
 public:
  template <typename Source, typename Predicate>
  void SetUp(Source&& source, Predicate&& predicate) {
    state_ = new ParserState(begin(std::forward<Source>(source)),
                             end(std::forward<Source>(source)));
    *state_ = *state_ | std::forward<Predicate>(predicate);
  }

  ~StringPipe() { delete state_; }

 protected:
  ParserState<InputIt>* state_;
};

TEST_F(StringPipe, IsDigitOk) {
  Buffer buf("1");
  SetUp(buf, is_digit_pipe<InputIt>);

  EXPECT_TRUE(state_->is_ok());
  EXPECT_FALSE(state_->has_next());
  EXPECT_EQ(buf, state_->buffer());
}

TEST_F(StringPipe, IsDigitError) {
  Buffer buf("x");
  SetUp(buf, is_digit_pipe<InputIt>);

  EXPECT_EQ(Status::kError, state_->status);
  EXPECT_TRUE(state_->has_next());
  EXPECT_TRUE(state_->buffer().empty());
}

TEST_F(StringPipe, IsDigitEOF) {
  Buffer buf("");
  SetUp(buf, is_digit_pipe<InputIt>);

  EXPECT_EQ(Status::kEOF, state_->status);
  EXPECT_FALSE(state_->has_next());
  EXPECT_TRUE(state_->buffer().empty());
}

TEST_F(StringPipe, IsZeroDigitOk) {
  Buffer buf("");
  SetUp(buf, is_zero_or_more_digits_pipe<InputIt>);

  EXPECT_TRUE(state_->is_ok());
  EXPECT_FALSE(state_->has_next());
  EXPECT_TRUE(state_->buffer().empty());
}

TEST_F(StringPipe, IsMoreDigitOk) {
  Buffer buf("01234567899876543210x");
  SetUp(buf, is_zero_or_more_digits_pipe<InputIt>);

  EXPECT_TRUE(state_->is_ok());
  EXPECT_TRUE(state_->has_next());
  EXPECT_EQ(Buffer("01234567899876543210"), state_->buffer());
  EXPECT_EQ('x', state_->next());
  EXPECT_FALSE(state_->has_next());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}