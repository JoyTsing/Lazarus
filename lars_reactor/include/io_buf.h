#pragma once

// buf_pool存储的单元

class io_buf {
 public:
  io_buf(int capacity);

  void clear();
  /**
   * @brief 已经处理len长度的数据
   *
   * @param len 已经处理的数据长度
   */
  void pop(int len);

  /**
   * @brief 把未处理的数据移到buf的头部，同时保留未处理的数据长度，
   * 同时调整length为未处理数据长度，head移动到0位置
   */
  void adjust();

  void copy(const io_buf* src);

  int capacity;  // buf的容积大小
  int length;    // buf中已经存储的数据大小
  int head;      // buf中未处理数据的起始位置
  char* data;    // buf的数据
  io_buf* next;  // 下一个io_buf
};