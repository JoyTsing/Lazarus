#pragma once

#include <cstdint>
#include <unordered_map>

#include "io_buf.h"

using bpool_t = std::unordered_map<int, IOBuffer*>;

enum class MEM_CAP : std::uint32_t {
  m4K = 4096,
  m16K = 16384,
  m64K = 65536,
  m256K = 262144,
  m1M = 1048576,
  m4M = 4194304,
  m8M = 8388608
};

class BufferPool {
 public:
  BufferPool(const BufferPool&) = delete;
  BufferPool& operator=(const BufferPool&) = delete;

  static BufferPool* instance() {
    static BufferPool* _instance;
    return _instance;
  }

 private:
  BufferPool();

 private:
  bpool_t _pool;
  std::uint64_t _total_mem;
};