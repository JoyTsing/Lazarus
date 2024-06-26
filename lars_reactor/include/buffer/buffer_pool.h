#pragma once

#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "buffer/io_buf.h"

using bpool_t = std::unordered_map<int, IOBuffer*>;

enum class MEM_CAP : std::uint32_t {
  m4K = 4096,      // 4K
  m16K = 16384,    // 16K
  m64K = 65536,    // 64K
  m256K = 262144,  // 256K
  m1M = 1048576,   // 1M
  m4M = 4194304,   // 4M
  m8M = 8388608    // 8M 23bit
};

inline const int mem_cap_to_int(MEM_CAP cap) { return static_cast<int>(cap); }

const constexpr std::uint64_t MEM_LIMIT =
    8368709120;  // 最初为5G，单位为B，再加3个G

class BufferPool {
 public:
  static std::shared_ptr<BufferPool> instance() {
    static auto _instance = std::shared_ptr<BufferPool>(new BufferPool);
    return _instance;
  }

  // 从内存池申请一个buffer
  IOBuffer* alloc_buffer();
  IOBuffer* alloc_buffer(int size);

  // 将buffer放回内存池
  void revert(IOBuffer* buf);

 private:
  BufferPool();

  BufferPool(const BufferPool&) = delete;
  BufferPool& operator=(const BufferPool&) = delete;

  void init_buffer_unit(int size, MEM_CAP page_size);

 private:
  bpool_t _pool;
  std::uint64_t _total_mem;
  std::mutex _mutex;
};