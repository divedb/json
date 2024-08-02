#pragma once

#include <cstdlib>
#include <set>

namespace json {

class MallocAllocator {
 public:
  MallocAllocator() = default;

  MallocAllocator(MallocAllocator const&) = delete;
  MallocAllocator& operator=(MallocAllocator const&) = delete;

  MallocAllocator(MallocAllocator&& other) noexcept
      : alloc_ptrs_(std::move(other.alloc_ptrs_)) {}
  MallocAllocator& operator=(MallocAllocator&& other) noexcept {
    alloc_ptrs_.swap(other.alloc_ptrs_);

    return *this;
  }

  ~MallocAllocator() {
    for (auto ptr : alloc_ptrs_) {
      ::free(ptr);
    }
  }

  void* malloc(size_t size) {
    void* ptr = ::malloc(size);
    alloc_ptrs_.insert(ptr);

    return ptr;
  }

 private:
  std::set<void*> alloc_ptrs_;
};

}  // namespace json