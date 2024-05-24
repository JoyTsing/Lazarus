#include "reactor_buf.h"

#include <sys/ioctl.h>
#include <unistd.h>

#include <cassert>
#include <cstring>
#include <iostream>

#include "buffer_pool.h"
#include "io_buf.h"

ReactorBuffer::ReactorBuffer() : _buffer(nullptr) {}

ReactorBuffer::~ReactorBuffer() { clear(); }

void ReactorBuffer::pop(int len) {
  if (_buffer == nullptr || len > _buffer->length) {
    return;
  }
  _buffer->pop(len);  // 我操了这里调成length了
  if (_buffer->length == 0) {
    BufferPool::instance()->revert(_buffer);
    _buffer = nullptr;  // 空数据的时候也能清空
  }
}

void ReactorBuffer::clear() {
  if (_buffer != nullptr) {
    BufferPool::instance()->revert(_buffer);
    _buffer = nullptr;
  }
}

int ReactorBuffer::length() {
  if (_buffer == nullptr) {
    return 0;
  }
  return _buffer->length;
}

int InputBuffer::read_data(int fd) {
  int need_read;
  if (ioctl(fd, FIONREAD, &need_read) == -1) {
    std::cerr << "ioctl error\n";
    return -1;
  }

  if (_buffer == nullptr) {
    _buffer = BufferPool::instance()->alloc_buffer(need_read);
    if (_buffer == nullptr) {
      return -1;
    }
  } else {
    assert(_buffer->head == 0);
    // 当前的buffer不够存了
    if (_buffer->capacity - _buffer->length < (int)need_read) {
      IOBuffer* new_buffer =
          BufferPool::instance()->alloc_buffer(need_read + _buffer->length);
      if (new_buffer == nullptr) {
        return -1;
      }
      new_buffer->copy(_buffer);
      BufferPool::instance()->revert(_buffer);  // 放回
      _buffer = new_buffer;
    }
  }
  // 读数据
  int read_len = 0;
  do {
    // 读取的数据拼接到之前的数据之后
    if (need_read == 0) {
      // 可能是阻塞，对方没写
      read_len = read(fd, _buffer->data + _buffer->length,
                      mem_cap_to_int(MEM_CAP::m4K));
    } else {
      read_len = read(fd, _buffer->data + _buffer->length, need_read);
    }
  } while (read_len == -1 && errno == EINTR);  // systemCall中断，重新读取

  if (read_len > 0) {
    if (need_read != 0) {
      assert(read_len == need_read);
    }
    _buffer->length += read_len;
  }
  return read_len;
}

const char* InputBuffer::data() {
  return _buffer != nullptr ? _buffer->data + _buffer->head : nullptr;
}

void InputBuffer::adjust() {
  if (_buffer != nullptr) {
    _buffer->adjust();
  }
}

int OutputBuffer::write_to(int fd) {
  assert(_buffer != NULL && _buffer->head == 0);
  int written = 0;
  do {
    written = write(fd, _buffer->data, _buffer->length);
  } while (written == -1 && errno == EINTR);

  if (written > 0) {
    _buffer->pop(written);
    _buffer->adjust();
  }

  // 非阻塞模式下，缓冲区满了，返回-1，errno=EAGAIN
  if (written == -1 && errno == EAGAIN) {
    written = 0;
  }
  return written;
}

int OutputBuffer::add_data(const char* data, int len) {
  if (_buffer == nullptr) {
    _buffer = BufferPool::instance()->alloc_buffer(len);
    if (_buffer == nullptr) {
      return -1;
    }
  } else {
    // 当前的buffer不够存了
    assert(_buffer->head == 0);
    if (_buffer->capacity - _buffer->length < len) {
      IOBuffer* new_buffer =
          BufferPool::instance()->alloc_buffer(len + _buffer->length);
      if (new_buffer == nullptr) {
        return -1;
      }
      new_buffer->copy(_buffer);
      BufferPool::instance()->revert(_buffer);  // 放回
      _buffer = new_buffer;
    }
  }
  // 将新数据拼接到之前的数据之后
  memcpy(_buffer->data + _buffer->length, data, len);
  _buffer->length += len;
  return 0;
}