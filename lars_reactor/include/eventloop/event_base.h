#pragma once

#include <functional>

class EventLoop;
using io_call_back = std::function<void(EventLoop* el, int fd, void* args)>;

#include "eventloop/event_loop.h"

#define IO_EVENT_ARGUMENT EventLoop *el, int fd, void *args

struct IOEvent {
  IOEvent()
      : mask(0),
        read_callback(nullptr),
        read_args(nullptr),
        write_callback(nullptr),
        write_args(nullptr) {}
  // 事件读写形式
  int mask;  // EPOLLIN,EPOLLOUT
  // 读事件回调函数
  io_call_back read_callback;
  // 读事件回调函数形参
  void* read_args;
  // 写事件回调函数
  io_call_back write_callback;
  // 写事件回调函数形参
  void* write_args;
};