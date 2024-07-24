#pragma once

#include <unistd.h>

#include <cassert>
#include <cctype>
#include <memory>
#include <string_view>
#include <vector>

#include "json/common/error.hpp"
#include "json/nodes/json_node.hpp"

namespace json {

struct Location {
  void update(int ch) {
    if (std::isspace(ch)) {
      y_pos++;
      x_pos = 0;
    } else {
      x_pos++;
    }

    source_location++;
  }

  int x_pos{};
  int y_pos{};
  int source_location{};
};

class CharStream {
 public:
  constexpr static char kInvalidChar{-1};

  virtual ~CharStream() = default;

  /// @brief Check if the stream has reached the end of file.
  ///
  /// @return True if the stream has no more characters to read, false
  ///         otherwise.
  constexpr bool is_eof() const { return !has_next(); }

  /// @brief Retrieve the next character from the stream.
  ///
  /// @return The next character in the stream.
  /// @throws std::runtime_error if the end of file is reached.
  constexpr char next_char() {
    if (one_look_ahead_ != kInvalidChar) {
      return std::exchange(one_look_ahead_, kInvalidChar);
    }

    char ch = next();
    update_location(ch);

    return ch;
  }

  /// @brief Retrieve the next n characters from the stream or EOF is reached.
  ///
  /// @param n The number of characters to retrieve.
  /// @return A vector containing the next n characters.
  virtual std::vector<char> next_nchars(int n) {
    std::vector<char> res;
    res.reserve(n);

    for (int i = 0; !is_eof() && i < n; i++) {
      res.push_back(next_char());
    }

    return res;
  }

  constexpr void put_back(char ch) {
    if (one_look_ahead_ != kInvalidChar) {
      THROW_ERROR("Only one look-ahead token is allowed in this context.");
    }

    one_look_ahead_ = ch;
  }

 protected:
  constexpr void update_location(char ch) {
    prev_token_loc_ = token_loc_;
    token_loc_.update(ch);
  }

  /// @brief Check if there are more characters to read in the stream.
  ///
  /// @return True if there are more characters to read, false otherwise.
  virtual bool has_next() const = 0;

  /// @brief Retrieve the next character from the stream without checking for
  ///        end of file.
  /// @return
  virtual char next() = 0;

 private:
  char one_look_ahead_{kInvalidChar};
  Location token_loc_;
  Location prev_token_loc_;
};

/// @brief A class that represents a character stream from memory.
class MemoryCharStream : public CharStream {
 public:
  /// @brief Constructs a MemoryCharStream from a std::string_view.
  ///
  /// @param data The data to be used as the character stream.
  explicit MemoryCharStream(std::string_view data) : data_{data} {}

  /// @brief Constructs a MemoryCharStream from a vector of characters.
  ///
  /// @param data The data to be used as the character stream.
  explicit MemoryCharStream(std::initializer_list<char> data) : data_{data} {}

  /// @brief Checks if there are more characters to read from the stream.
  ///
  /// @return True if there are more characters, otherwise false.
  bool has_next() const override { return remain() > 0; }

  /// @brief Reads the next character from the stream.
  ///
  /// @return The next character in the stream.
  /// @throws An error if attempting to read beyond the end of the stream.
  char next() override {
    if (!has_next()) {
      THROW_ERROR(
          "Reached end of file while attempting to read the next character.");
    }

    return data_[pos_++];
  }

  /// @brief Reads the next n characters from the stream.
  ///
  /// @param n The number of characters to read.
  /// @return A vector containing the next n characters.
  std::vector<char> next_nchars(int n) override {
    int count = std::min(n, remain());
    std::vector<char> res(count);
    std::copy_n(data_.begin() + pos_, count, res.begin());
    pos_ += count;

    // TODO: SSE or AVX

    return res;
  }

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
  explicit FdCharStream(int fd, bool close_fd = true)
      : fd_{fd}, close_fd_{close_fd} {
    assert(fd_ >= 0);
  }

  ~FdCharStream() {
    if (close_fd_) {
      ::close(fd_);
    }
  }

  /// @brief Checks if there are more characters to read from the stream.
  ///
  /// @return True if there are more characters, otherwise false.
  bool has_next() const override { return false; }

  /// @brief Reads the next character from the stream.
  ///
  /// @return The next character in the stream.
  /// @throws An error if attempting to read beyond the end of the stream.
  char next() override { return 0; }
  std::vector<char> next_nchars(int n) override { return {}; }

 private:
  int fd_;
  bool close_fd_;
};

class ParserState {
 public:
  constexpr bool is_ok() { return is_ok_; }

  constexpr void set_error(std::string const& error) {
    error_ = error;
    is_ok_ = false;
  }

 private:
  bool is_ok_{true};
  std::string error_;
  Location token_loc_;
  Location prev_token_loc_;
};

class MemoryContetx;

class Parser {
 public:
  explicit Parser(MemoryContetx& mem_ctx) : mem_ctx_{mem_ctx} {}

  JsonNode* parse(CharStream& stream, ParserState& state);

 private:
  JsonNode* parse_null(CharStream& stream, ParserState& state);
  JsonNode* parse_bool(CharStream& stream, ParserState& state);
  JsonNode* parse_number(CharStream& stream, ParserState& state);
  JsonNode* parse_string(CharStream& stream, ParserState& state);
  JsonNode* parse_array(CharStream& stream, ParserState& state);
  JsonNode* parse_object(CharStream& stream, ParserState& state);

  MemoryContetx& mem_ctx_;
};

}  // namespace json