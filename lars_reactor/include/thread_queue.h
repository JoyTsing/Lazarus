#pragma once
#include <sys/eventfd.h>
#include <unistd.h>

#include <cstdio>
#include <mutex>
#include <queue>

#include "event_base.h"
#include "event_loop.h"

template <typename T>
class ThreadQueue {
 public:
  ThreadQueue() : _loop(nullptr) {
    // 创建一个event_fd用于监听，没有跟socket和文件描述符关联
    _fd = eventfd(0, EFD_NONBLOCK);
    if (_fd == -1) {
      perror("eventfd() create error");
      exit(1);
    }
  }

  ~ThreadQueue() {
    if (_fd != -1) {
      close(_fd);
    }
  }

  // 添加任务
  void send(const T& task) {
    std::lock_guard<std::mutex> lock(_mtx);
    _queue.push(task);
    // 通知event_fd有数据
    uint64_t one = 1;
    int ret = write(_fd, &one, sizeof(one));
    if (ret == -1) {
      perror("write() error");
      exit(1);
    }
  }

  /**
   * @brief 获取队列，被_fd激活的read event时候调用
   *
   * @param queue 需要为空的队列
   */
  void recv(std::queue<T>& queue) {
    std::lock_guard<std::mutex> lock(_mtx);
    // fd绑定的事件读缓存清空,将占位的数据读出
    uint64_t one;
    int ret = read(_fd, &one, sizeof(one));
    if (ret == -1) {
      perror("read() error");
      exit(1);
    }
    // 交换队列
    std::swap(queue, _queue);
  }

  void set_loop(EventLoop* loop) { _loop = loop; }

  // 设置_fd触发时的回调函数
  void set_callback(io_call_back cb, void* args = nullptr) {
    if (_loop == nullptr) {
      return;
    }
    _loop->add_io_event(_fd, EPOLLIN, cb, args);
  }

 private:
  int _fd;
  EventLoop* _loop;
  std::queue<T> _queue;
  std::mutex _mtx;
};