#pragma once

#include <sys/epoll.h>

#include <unordered_set>
#include <vector>

#include "eventloop/event_base.h"
#include "message/task_message.h"

struct Task;
struct IOEvent;

using io_event_map = std::unordered_map<int, IOEvent>;
using io_event_map_iter = io_event_map::iterator;
using listen_fd_set = std::unordered_set<int>;

class EventLoop {
 public:
  EventLoop();
  ~EventLoop();
  /**
   * @brief
   * 阻塞循环监听事件发生,并处理epoll_wait返回的事件，同时调用对应的回调函数
   *
   */
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

  static constexpr const int MAX_EVENTS = 1024;

  // 添加一个task任务到ready_tasks
  void add_task(const Task& task);

  // 执行ready_tasks中的所有任务,执行完后清空ready_tasks
  void process_tasks();

  // 获取在线的fd
  const listen_fd_set& get_listen_fds() const;

 private:
  int _epoll_fd;  // epoll_create
  // 事件表
  io_event_map _io_events;
  // 正在监听的fd集合
  listen_fd_set _listen_fds;
  // 每次epoll_wait返回的事件集合
  epoll_event _fired_events[MAX_EVENTS];
  // 异步task任务: 回调函数及其参数
  std::vector<Task> _ready_tasks;
};
