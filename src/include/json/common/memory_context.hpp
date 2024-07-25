#pragma once

#include <cstddef>
#include <memory>
#include <set>
#include <vector>

#include "json/common/error.hpp"
#include "json/common/util.hpp"

namespace json {

/// @brief Manage memory allocation and deallocation. Note that, this class
///        handles memory deallocation automatically when allocated bytes is
///        less than 4096-16 = 4080 bytes. If it's a large malloc, the client
///        needs to free manually.
class MemoryContext {
 public:
  static constexpr int kPageSize = 4096;
  static constexpr int kFreeChunks = 10;

  template <typename T, typename... Args>
  T* create(Args&&... args) {
    void* ptr = malloc(sizeof(T));

    return new (ptr) T(static_cast<Args&&>(args)...);
  }

  /// @brief Constructs a MemoryContext object.
  MemoryContext() : free_chunks_(kFreeChunks, nullptr) {}

  MemoryContext(MemoryContext const&) = delete;
  MemoryContext& operator=(MemoryContext const&) = delete;

  MemoryContext(MemoryContext&& other) noexcept
      : used_blocks_(std::move(other.used_blocks_)),
        free_chunks_(std::move(other.free_chunks_)) {}

  MemoryContext& operator=(MemoryContext&& other) noexcept {
    MemoryContext().swap(*this);
    swap(other);

    return *this;
  }

  ~MemoryContext() {
    for (auto& block : used_blocks_) {
      ::free(block.begin_);
    }
  }

  void swap(MemoryContext& other) {
    used_blocks_.swap(other.used_blocks_);
    free_chunks_.swap(other.free_chunks_);
  }

  /// @brief Allocates memory.
  ///
  /// @param nbytes Number of bytes to allocate.
  /// @return Pointer to the allocated memory.
  void* malloc(size_t nbytes) {
    Chunk* chunk = nullptr;
    size_t alloc_size = aligned_alloc_size(nbytes);

    if (alloc_size > kPageSize) {
      chunk = malloc_large(alloc_size);
    } else {
      // The chunk in freelist must be aligned.
      chunk = try_allocate_from_free_chunks(nbytes);

      if (!chunk) {
        chunk = try_allocate_from_used_blocks(alloc_size);
      }

      if (!chunk) {
        chunk = allocate_pagesz_block(alloc_size);
      }
    }

    assert(chunk);

    return Chunk::cast_chunk(chunk);
  }

  /// @brief Reallocates memory.
  ///
  /// @param ptr Pointer to the existing memory.
  /// @param new_size New size for the memory allocation.
  /// @return Pointer to the reallocated memory.
  void* realloc(void* ptr, size_t new_size) {
    Chunk* chunk = Chunk::cast_ptr(ptr);

    check_magic(chunk);

    // If the remain memory is enough.
    if (chunk->size >= new_size) {
      return ptr;
    }

    size_t alloc_size = aligned_alloc_size(chunk->size);

    // If this is a large malloc, we need to allocate larger space.
    if (alloc_size > kPageSize) {
      new_size = aligned_alloc_size(new_size);
      ptr = ::realloc(chunk, new_size);

      return new (ptr) Chunk(new_size, magic());
    }

    // Otherwise, this is a small malloc.
    void* new_ptr = malloc(new_size);
    std::memcpy(new_ptr, ptr, chunk->size);
    free_chunk(chunk);

    return new_ptr;
  }

  /// @brief Frees allocated memory.
  ///
  /// @param ptr Pointer to the memory to free.
  void free(void* ptr) {
    Chunk* chunk = Chunk::cast_ptr(ptr);
    check_magic(chunk);
    free_chunk(chunk);
  }

 private:
  class Chunk {
   public:
    static constexpr size_t chunk_header_size() { return sizeof(Chunk); }

    /// @brief Casts a pointer to a Chunk pointer.
    ///
    /// @param ptr Pointer to cast.
    /// @return Chunk pointer.
    static Chunk* cast_ptr(void* ptr) {
      return reinterpret_cast<Chunk*>(static_cast<char*>(ptr) -
                                      chunk_header_size());
    }

    /// @brief Casts a Chunk pointer to a void pointer.
    ///
    /// @param chunk Chunk pointer to cast.
    /// @return Void pointer.
    static void* cast_chunk(Chunk* chunk) {
      return static_cast<void*>(reinterpret_cast<char*>(chunk) +
                                chunk_header_size());
    }

    /// @brief Constructs a Chunk object.
    ///
    /// @param chunk_size Size of the chunk.
    /// @param magic Magic pointer for validation.
    Chunk(size_t chunk_size, void* magic)
        : size(chunk_size - chunk_header_size()), next(magic) {}

    size_t size;
    void* next;  ///< Next or magic
  };

  class Block {
   public:
    friend class MemoryContext;

    /// @brief Constructs a Block object.
    ///
    /// @param ptr Pointer to the start of the block.
    /// @param size Size of the block.
    Block(void* ptr, size_t size)
        : begin_(ptr), end_(ptr), cap_(static_cast<char*>(ptr) + size) {}

    // /// @brief Destructs the Block object and frees the memory.
    // ~Block() { ::free(begin_); }

    /// @brief Allocates memory within the block.
    ///
    /// @param size Size of memory to allocate.
    /// @return Pointer to the allocated memory.
    void* allocate(size_t size) {
      if (remaining() < size) {
        return nullptr;
      }

      return std::exchange(end_, static_cast<char*>(end_) + size);
    }

    /// @brief Gets the remaining size in the block.
    ///
    /// @return Remaining size.
    constexpr size_t remaining() const {
      return static_cast<char*>(cap_) - static_cast<char*>(end_);
    }

   private:
    void* begin_;
    void* end_;
    void* cap_;
  };

  /// @brief Calculates the size including the chunk header.
  ///
  /// @param nbytes Number of bytes to allocate.
  /// @return Size including the chunk header.
  size_t size_with_chunk_header(size_t nbytes) {
    return nbytes + Chunk::chunk_header_size();
  }

  /// @brief Aligns the allocation size to the next power of 2.
  ///
  /// @param nbytes Number of bytes to allocate.
  /// @return Aligned allocation size.
  size_t aligned_alloc_size(size_t nbytes) {
    size_t min_size = size_with_chunk_header(nbytes);

    return next_power_of_2(static_cast<unsigned int>(min_size));
  }

  /// @brief Allocates large memory chunks.
  ///
  /// @param alloc_size Size of the allocation.
  /// @return Pointer to the allocated Chunk.
  Chunk* malloc_large(size_t alloc_size) {
    void* ptr = ::malloc(alloc_size);

    return new (ptr) Chunk(alloc_size, magic());
  }

  /// @brief Tries to allocate memory from free chunks.
  ///
  /// @param size Size of memory to allocate.
  /// @return Pointer to the allocated Chunk or nullptr if not available.
  Chunk* try_allocate_from_free_chunks(size_t size) {
    int index = free_index(size);

    Chunk* prev = nullptr;
    Chunk* chunk = free_chunks_[index];

    while (chunk) {
      Chunk* next = static_cast<Chunk*>(chunk->next);

      if (chunk->size >= size) {
        if (prev == nullptr) {
          free_chunks_[index] = next;
        } else {
          prev->next = next;
        }

        chunk->next = magic();

        return chunk;
      }

      chunk = next;
    }

    return nullptr;
  }

  /// @brief Tries to allocate memory from used blocks.
  ///
  /// @param size Size of memory to allocate.
  /// @return Pointer to the allocated Chunk or nullptr if not available.
  Chunk* try_allocate_from_used_blocks(size_t size) {
    for (auto& block : used_blocks_) {
      void* ptr = block.allocate(size);

      if (ptr != nullptr) {
        return new (ptr) Chunk(size, magic());
      }
    }

    return nullptr;
  }

  /// @brief Allocates a new block of memory of page size.
  ///
  /// @param size Size of memory to allocate.
  /// @return Pointer to the allocated Chunk.
  Chunk* allocate_pagesz_block(size_t size) {
    assert(size <= kPageSize);

    void* ptr = ::malloc(kPageSize);
    used_blocks_.emplace_back(ptr, kPageSize);
    Block& block = used_blocks_.back();
    ptr = block.allocate(size);

    return new (ptr) Chunk(size, magic());
  }

  /// @brief Gets the magic pointer for validation.
  ///
  /// @return Magic pointer.
  void* magic() { return this; }

  /// @brief Checks the magic pointer for validation.
  ///
  /// @param chunk Chunk to check.
  void check_magic(Chunk const* chunk) {
    if (chunk->next != magic()) {
      THROW_ERROR("Pointer freed is not within this context");
    }
  }

  /// @brief Frees a chunk of memory.
  ///
  /// @param chunk Chunk to free.
  void free_chunk(Chunk* chunk) {
    size_t size = chunk->size;
    int index = free_index(size);
    chunk->next = free_chunks_[index];
    free_chunks_[index] = chunk;
  }

  /// @brief Gets the free index for a given size.
  ///
  /// @param size Size of memory.
  /// @return Free index.
  int free_index(size_t size) const {
    assert(size <= kPageSize);

    const int sz = 8;

    for (int i = 0; i < kFreeChunks; i++) {
      if ((sz << i) >= size) {
        return i;
      }
    }

    return -1;
  }

  std::vector<Block> used_blocks_;

  // Chunk 1:  (0,    8]    bytes
  // Chunk 2:  (8,    16]   bytes
  // Chunk 3:  (16,   32]   bytes
  // Chunk 4:  (32,   64]   bytes
  // Chunk 5:  (64,   128]  bytes
  // Chunk 6:  (128,  256]  bytes
  // Chunk 7:  (256,  512]  bytes
  // Chunk 8:  (512,  1024] bytes
  // Chunk 9:  (1024, 2048] bytes
  // Chunk 10: (2048, 4096] bytes
  std::vector<Chunk*> free_chunks_;
};

}  // namespace json