#pragma once

#include <vector>
#include <unistd.h>

#include "json/common/error.hpp"

namespace json {

class CharStream {
 public:
  constexpr static char kInvalidChar{-1};

  virtual ~CharStream() = default;

  /// @brief Check if the stream has reached the end of file.
  ///
  /// @return True if the stream has no more characters to read, false
  ///         otherwise.
  virtual bool is_eof() const = 0;

  /// @brief Retrieve the next character from the stream.
  ///
  /// @return The next character in the stream.
  /// @throws std::runtime_error if the end of file is reached.
  virtual char next_char() = 0;

  /// @brief Peek at the next character in the stream without advancing the
  ///        position.
  ///
  /// @return The next character in the stream.
  virtual char peek() = 0;

  /// @brief Retrieve the next n characters from the stream or EOF is reached.
  ///
  /// @param n The number of characters to retrieve.
  /// @return A vector containing the next n characters.
  virtual std::vector<char> next_nchars(int n) = 0;

  virtual void putback(char ch) = 0;

 protected:
  void throw_if_eof() const {
    if (is_eof()) {
      THROW_ERROR("Reached EOF while attempting to read.");
    }
  }
};

/// @brief A class that represents a character stream from memory.
class MemoryCharStream : public CharStream {
 public:
  /// @brief Constructs a MemoryCharStream from a std::string_view.
  ///
  /// @param data The data to be used as the character stream.
  explicit MemoryCharStream(std::string_view data)
      : data_{data.begin(), data.end()} {}

  /// @brief Constructs a MemoryCharStream from a vector of characters.
  ///
  /// @param data The data to be used as the character stream.
  explicit MemoryCharStream(std::initializer_list<char> data) : data_{data} {}

  bool is_eof() const override { return remain() == 0; }

  char next_char() override {
    throw_if_eof();

    return data_[pos_++];
  }

  char peek() override {
    throw_if_eof();

    return data_[pos_];
  }

  std::vector<char> next_nchars(int n) override {
    const int count = std::min(n, remain());
    std::vector<char> res(count);
    std::copy_n(data_.begin() + pos_, count, res.begin());
    pos_ += count;

    // TODO: SSE or AVX

    return res;
  }

  void putback(char ch) override {}

 private:
  /// @brief Calculates the number of remaining characters in the stream.
  ///
  /// @return The number of characters remaining to be read.
  int remain() const { return data_.size() - pos_; }

  int pos_{};
  std::vector<char> data_;
};

class FdCharStream : public CharStream {
 public:
  static constexpr char kInvalidChar{-1};

  explicit FdCharStream(int fd) : fd_{fd}, one_look_ahead_{kInvalidChar} {
    assert(fd_ >= 0);
  }

  bool is_eof() const override {
    if (one_look_ahead_ != kInvalidChar) {
      return false;
    }

    // If successful, the number of bytes actually read is returned.  Upon
    // reading end of file, zero is returned.Otherwise, -1 is returned
    // and the global variable errno is set to indicate the error.
    int n = read(fd_, &one_look_ahead_, 1);

    return n == 0;
  }

  char next_char() override {
    throw_if_eof();

    return std::exchange(one_look_ahead_, kInvalidChar);
  }

  char peek() override {
    throw_if_eof();

    return one_look_ahead_;
  }

  std::vector<char> next_nchars(int n) override {
    std::vector<char> res;
    res.reserve(n);

    for (int i = 0; !is_eof() && i < n; i++) {
      res.push_back(next_char());
    }

    return res;
  }

  void putback(char ch) override {}

 private:
  int fd_;
  mutable char one_look_ahead_;
};

}