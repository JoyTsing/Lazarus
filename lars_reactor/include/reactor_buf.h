#pragma once

#include "io_buf.h"
class ReactorBuffer {
 public:
  ReactorBuffer();
  virtual ~ReactorBuffer();

  void pop(int len);
  void clear();
  /**
   * @brief 返回当前还有多少有效数据
   *
   * @return int
   */
  int length();

 protected:
  IOBuffer* _buffer;
};

// 读buffer
class InputBuffer : public ReactorBuffer {
 public:
  int read_data(int fd);

  const char* data();
  /**
   * @brief 重置缓冲区
   *
   */
  void adjust();
};

// 写buffer
class OutputBuffer : public ReactorBuffer {
 public:
  int write_to(int fd);
  int add_data(const char* data, int len);
};