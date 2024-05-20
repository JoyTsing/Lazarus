#include "buffer_pool.h"

#include <cassert>
#include <iostream>
#include <mutex>

#include "io_buf.h"

void BufferPool::init_buffer_unit(int size, MEM_CAP page_size) {
  IOBuffer* prev = nullptr;
  int mem_size = static_cast<int>(page_size);
  _pool[mem_size] = new IOBuffer(mem_size);
  assert(_pool[mem_size]);
  prev = _pool[mem_size];
  for (int i = 1; i < size; i++) {
    prev->next = new IOBuffer(mem_size);
    assert(prev->next);
    prev->next = prev;
  }
  _total_mem += size * mem_size;
}

BufferPool::BufferPool() : _total_mem(0) {
  // 开辟内存池
  // 4K 5000个, 20M
  init_buffer_unit(5000, MEM_CAP::m4K);
  // 16K 1000个, 16M
  init_buffer_unit(1000, MEM_CAP::m16K);
  // 64K 500个, 32M
  init_buffer_unit(500, MEM_CAP::m64K);
  // 256K 200个, 50M
  init_buffer_unit(200, MEM_CAP::m256K);
  // 1M 50个, 50M
  init_buffer_unit(50, MEM_CAP::m1M);
  // 4M 20个, 80M
  init_buffer_unit(20, MEM_CAP::m4M);
  // 8M 10个, 80M
  init_buffer_unit(10, MEM_CAP::m8M);
}

IOBuffer* BufferPool::alloc_buffer(int size) {
  // 从内存池中找到合适的内存块
  int mem_size = 0;
  if (size <= mem_cap_to_int(MEM_CAP::m4K)) {
    mem_size = mem_cap_to_int(MEM_CAP::m4K);
  } else if (size <= mem_cap_to_int(MEM_CAP::m16K)) {
    mem_size = mem_cap_to_int(MEM_CAP::m16K);
  } else if (size <= mem_cap_to_int(MEM_CAP::m64K)) {
    mem_size = mem_cap_to_int(MEM_CAP::m64K);
  } else if (size <= mem_cap_to_int(MEM_CAP::m256K)) {
    mem_size = mem_cap_to_int(MEM_CAP::m256K);
  } else if (size <= mem_cap_to_int(MEM_CAP::m1M)) {
    mem_size = mem_cap_to_int(MEM_CAP::m1M);
  } else if (size <= mem_cap_to_int(MEM_CAP::m4M)) {
    mem_size = mem_cap_to_int(MEM_CAP::m4M);
  } else if (size <= mem_cap_to_int(MEM_CAP::m8M)) {
    mem_size = mem_cap_to_int(MEM_CAP::m8M);
  } else {
    return nullptr;
  }

  // 检查内存池中是否有足够的内存,没有则申请
  std::lock_guard<std::mutex> lock(_mutex);
  [[unlikely]] if (_pool[mem_size] == nullptr) {
    if (_total_mem + mem_size > MEM_LIMIT) {
      std::cerr << "buffer pool no enough memory\n";
      return nullptr;
    }
    IOBuffer* buf = new IOBuffer(mem_size);
    assert(buf);
    _total_mem += mem_size;
    return buf;
  }
  // 从内存池中取出一个内存块
  IOBuffer* buf = _pool[mem_size];
  _pool[mem_size] = buf->next;
  buf->next = nullptr;
  return buf;
}

IOBuffer* BufferPool::alloc_buffer() {
  return alloc_buffer(mem_cap_to_int(MEM_CAP::m4K));
}

void BufferPool::revert(IOBuffer* buf) {
  int mem_size = buf->capacity;
  buf->clear();
  std::lock_guard<std::mutex> lock(_mutex);
  assert(_pool.find(mem_size) != _pool.end());
  // 将内存块放回内存池
  buf->next = _pool[mem_size];
  _pool[mem_size] = buf;
}
