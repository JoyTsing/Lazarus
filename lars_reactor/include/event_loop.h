#pragma once

#include <sys/epoll.h>

#include <unordered_set>
#include <vector>

#include "event_base.h"

struct IOEvent;

using io_event_map = std::unordered_map<int, IOEvent>;
using io_event_map_iter = io_event_map::iterator;
using listen_fd_set = std::unordered_set<int>;

class EventLoop {
 public:
  EventLoop();

  // 阻塞等待事件发生
  void event_process();

  /**
   * @brief
   *  添加事件
   * @param fd
   * @param mask 对应的事件触发条件(EPOLLIN,EPOLLOUT)
   * @param proc 触发事件时的回调函数
   * @param args 回调函数参数
   */
  void add_io_event(int fd, int mask, io_call_back proc, void* args);

  // 删除事件
  void del_io_event(int fd);

  // 删除事件触发条件(EPOLLIN,EPOLLOUT)
  void del_io_event(int fd, int mask);

 private:
  static constexpr const int MAX_EVENTS = 128;

  int _epoll_fd;  // epoll_create
  // 事件表
  io_event_map _io_events;
  // 正在监听的fd集合
  listen_fd_set _listen_fds;
  // 每次epoll_wait返回的事件集合
  std::vector<epoll_event> _fired_events;
};