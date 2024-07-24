#pragma once

#include <string_view>
#include <type_traits>

#include "json/common/error.hpp"

namespace json {

/// @brief A char buffer is a linear, finite sequence of elements of char.
///
///        A buffer's limit is the index of the first element that should not be
///        read or written. A buffer's limit is never negative and is never
///        greater than its capacity.
///
///        A buffer's position is the index of the next element to be read or
///        written. A buffer's position is never negative and is never greater
///        than its limit.
///
/// @tparam N
template <typename T, std::size_t N>
class Buffer {
 public:
  Buffer() : limit_{N} {}

  template <typename InputIt,
            std::enable_if_t<
                std::is_same_v<std::iterator_traits<InputIt>::value_type, T>,
                int> = 0>
  Buffer(InputIt first, InputIt last) {
    auto dist = std::distance(first, last);

    assert(dist <= N && dist >= 0);

    limit_ = dist;
  }

  /// @brief Returns this buffer's position.
  ///
  /// @return The position of this buffer
  constexpr int position() const { return position_; }

  /// @brief Returns this buffer's limit.
  ///
  /// @return The limit of this buffer
  constexpr int limit() const { return limit_; }

  /// @brief Sets this buffer's mark at its position.
  ///
  /// @return This buffer
  constexpr Buffer& mark() {
    mark_ = position_;

    return *this;
  }

  /// @brief Resets this buffer's position to the previously-marked position.
  ///        Invoking this method neither changes nor discards the mark's value.
  ///
  /// @return This buffer
  constexpr Buffer& reset() {
    int m = mark_;

    if (m < 0) {
      THROW_ERROR("Invalid mark");
    }

    position_ = m;

    return *this;
  }

  /// @brief Clears this buffer. The position is set to zero, the limit is set
  ///        to the capacity, and the mark is discarded.
  ///        Invoke this method before using a sequence of channel-read or
  ///        put operations to fill this buffer.
  ///
  constexpr Buffer& clear() {
    position_ = 0;
    limit_ = N;
    mark_ = -1;

    return *this;
  }

  /// @brief Flips this buffer. The limit is set to the current position and
  ///        then the position is set to zero. If the mark is defined then it
  ///        is discarded.
  ///
  ///        <p> After a sequence of channel-read or <i>put</i> operations,
  ///        invoke this method to prepare for a sequence of channel-write or
  ///        relative <i>get</i> operations. For example:
  ///
  /// @code
  /// buf.put(magic);   // Prepend header
  /// in.read(buf);     // Read data into rest of buffer
  /// buf.flip();       // Flip buffer
  /// out.write(buf);   // Write header + data to channel
  /// @endcode
  /// @return
  constexpr Buffer& flip() {
    limit_ = position_;
    position_ = 0;
    mark_ = -1;

    return *this;
  }

  /// @brief Rewinds this buffer. The position is set to zero and the mark is
  ///        discarded.
  ///        <p> Invoke this method before a sequence of channel-write or
  ///        <i>get</i> operations, assuming that the limit has already been set
  ///        appropriately. For example:
  ///
  /// @code
  /// out.write(buf);   // Write remaining data
  /// buf.rewind();     // Rewind buffer
  /// buf.get(array);   // Copy data into array
  /// @endcode
  ///
  /// @return This buffer
  constexpr Buffer& rewind() {
    position_ = 0;
    mark_ = -1;

    return *this;
  }

  /// @brief Returns the number of elements between the current position and the
  ///        limit.
  ///
  /// @return The number of elements remaining in this buffer
  constexpr int remaining() const {
    int rem = limit_ - position_;

    return rem > 0 ? rem : 0;
  }

  /// @brief Tells whether there are any elements between the current position
  ///        and the limit.
  ///
  /// @return True if, and only if, there is at least one element remaining in
  ///         this buffer
  constexpr bool has_remaining() const { return position_ < limit_; }

  constexpr void discard_mark() { mark_ = -1; }

  void append(std::initializer_list<T> data) {
    assert(remaining() >= data.size());
  }

  void append(T const& data) {}
  void append(Buffer const& buf);

  T& operator[](std::size_t index) {
    assert(index < limit_);

    return data_[index];
  }

  T const& operator[](std::size_t index) const {
    assert(index < limit_);

    return data_[index];
  }

  T* begin() { return ::begin(data_); }
  T const* cbegin() { return ::begin(data_); }
  T* end() { return ::end(data_); }
  T const* cend() { return ::end(data_); }

  void swap(Buffer& other) noexcept {}

 private:
  int mark_{-1};
  int position_{};
  int limit_{N};
  T data_[N];
};

}  // namespace json