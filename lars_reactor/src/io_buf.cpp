#include "io_buf.h"

#include <cassert>
#include <cstring>

io_buf::io_buf(int capacity)
    : capacity(capacity), length(0), head(0), data(new char[capacity]) {
  next = nullptr;
  assert(data);
}

void io_buf::clear() {
  length = 0;
  head = 0;
}

void io_buf::pop(int len) {
  head += len;
  length -= len;
}

void io_buf::adjust() {
  length -= head;
  if (length > 0) {
    memmove(data, data + head, length);
  }
  head = 0;
}

void io_buf::copy(const io_buf* src) {
  length = src->length;
  head = 0;
  memcpy(data, src->data + src->head, src->length);
}
