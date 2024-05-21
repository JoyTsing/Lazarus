#include "io_buf.h"

#include <cassert>
#include <cstring>

IOBuffer::IOBuffer(int capacity)
    : capacity(capacity), length(0), head(0), data(new char[capacity]) {
  next = nullptr;
  assert(data);
}

void IOBuffer::clear() {
  length = 0;
  head = 0;
}

void IOBuffer::pop(int len) {
  head += len;
  length -= len;
}

void IOBuffer::adjust() {
  if (head != 0) {
    if (length != 0) {
      memmove(data, data + head, length);
    }
    head = 0;
  }
}

void IOBuffer::copy(const IOBuffer* src) {
  memcpy(data, src->data + src->head, src->length);
  length = src->length;
  head = 0;
}
