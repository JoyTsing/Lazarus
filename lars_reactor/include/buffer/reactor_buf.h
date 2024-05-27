#pragma once

#include "buffer/io_buf.h"
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
  /**
   * @brief 从指定的socket中读数据，-1表示错误
   *
   * @param fd
   * @return int
   */
  int read_data(int fd);

  /**
   * @brief 返回缓冲区的数据
   *
   * @return const char*
   */
  const char* data();

  /**
   * @brief 将缓冲区已读取数据剔除，需要配合pop使用
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