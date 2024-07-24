
#include "json/common/memory_context.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <random>

using namespace std;
using namespace json;

TEST(MemoryContext, MemoryLeakDetection) {
  vector<int> size;
  MemoryContext mem_ctx;
  std::random_device rd;
  std::mt19937 g(rd());

  for (int i = 1; i <= 512; i++) {
    size.push_back(i);
  }

  for (int i = 0; i < 10; i++) {
    for (auto s : size) {
      mem_ctx.malloc(s);
    }

    shuffle(size.begin(), size.end(), g);
  }
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}