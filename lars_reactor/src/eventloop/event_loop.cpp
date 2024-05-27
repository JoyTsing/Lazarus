#include "eventloop/event_loop.h"

#include <sys/epoll.h>

#include <cassert>
#include <cstddef>

#include "eventloop/event_base.h"
#include "utils/minilog.h"

EventLoop::EventLoop() {
  _epoll_fd = epoll_create1(0);
  if (_epoll_fd == -1) {
    minilog::log_error("EventLoop::EventLoop epoll_create1 failed");
    exit(1);
  }
}

void EventLoop::event_process() {
  while (true) {
    io_event_map_iter it;
    int nfds = epoll_wait(_epoll_fd, _fired_events, MAX_EVENTS, -1);
    for (int i = 0; i < nfds; i++) {
      int fd = _fired_events[i].data.fd;
      it = _io_events.find(fd);
      assert(it != _io_events.end());
      IOEvent* event = &(it->second);
      if (_fired_events[i].events & EPOLLIN) {
        // read
        void* args = event->read_args;
        event->read_callback(this, fd, args);
      } else if (_fired_events[i].events & EPOLLOUT) {
        // write
        void* args = event->write_args;
        event->write_callback(this, fd, args);
      } else if (_fired_events[i].events & (EPOLLERR | EPOLLHUP)) {
        // 水平触发未处理出现HUP，需要正常处理读写，如果events没有读也没有写，将events从epoll中去掉
        if (event->read_callback != nullptr) {
          void* args = event->read_args;
          event->read_callback(this, fd, args);
        } else if (event->write_callback != nullptr) {
          void* args = event->write_args;
          event->write_callback(this, fd, args);
        } else {
          // 从epoll中去掉
          minilog::log_error(
              "EventLoop::event_process EPOLLERR | EPOLLHUP, remove fd: {}",
              fd);
          del_io_event(fd);
        }
      }
    }
  }
}
void EventLoop::add_io_event(int fd, int mask, io_call_back proc, void* args) {
  int op, final_mask;
  auto it = _io_events.find(fd);
  // 1是否已有事件,不存在则添加，存在则修改
  if (it == _io_events.end()) {
    op = EPOLL_CTL_ADD;
    final_mask = mask;
  } else {
    op = EPOLL_CTL_MOD;
    final_mask = it->second.mask | mask;
  }
  // 2绑定fd和callback
  if (mask & EPOLLIN) {
    // 读事件
    _io_events[fd].read_callback = proc;
    _io_events[fd].read_args = args;
  } else if (mask & EPOLLOUT) {
    // 写事件
    _io_events[fd].write_callback = proc;
    _io_events[fd].write_args = args;
  }

  // 更新mask
  _io_events[fd].mask = final_mask;
  // 3添加到原生的epoll

  epoll_event ev;
  ev.data.fd = fd;
  ev.events = final_mask;
  if (epoll_ctl(_epoll_fd, op, fd, &ev) == -1) {
    minilog::log_error("EventLoop::add_io_event epoll_ctl failed");
    return;
  }
  // 4添加到正在监听的fd中
  _listen_fds.insert(fd);
}

void EventLoop::del_io_event(int fd) {
  // 从事件表中删除
  _io_events.erase(fd);
  _listen_fds.erase(fd);
  // 从epoll中删除
  epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, NULL);
}

void EventLoop::del_io_event(int fd, int mask) {
  auto it = _io_events.find(fd);
  if (it == _io_events.end()) {
    return;
  }
  int& final_mask = it->second.mask;
  final_mask = final_mask & (~mask);
  if (final_mask == 0) {
    minilog::log_info("EventLoop::del_io_event {}", fd);
    del_io_event(fd);
  } else {
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = final_mask;
    epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, fd, &ev);
  }
}
