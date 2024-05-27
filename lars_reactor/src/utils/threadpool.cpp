#include "utils/threadpool.h"

#include <iostream>
#include <thread>

#include "eventloop/event_loop.h"
#include "message/task_message.h"
#include "net/tcp/tcp_connection.h"

/**
 * @brief 监听task是否收到消息，收到消息则读出消息队列里的消息并进行处理
 *
 * @param el
 * @param fd
 * @param args
 */

void deal_task(EventLoop* el, int fd, void* args) {
  // 从消息队列中读取消息
  ThreadQueue<task_message>* queue = (ThreadQueue<task_message>*)args;
  std::queue<task_message> task_queue;
  queue->recv(task_queue);
  while (!task_queue.empty()) {
    task_message task = task_queue.front();
    task_queue.pop();
    // 判断消息类型
    switch (task.type) {
      case task_message::TaskType::NEW_CONNECTION: {
        // 新连接
        TCPConnection* conn = new TCPConnection(std::get<int>(task.data), el);
        // 创建链接，同时讲这个链接加入到event_loop中
        if (conn == nullptr) {
          std::cerr << "new connection error" << std::endl;
          break;
        }
        std::cout << "new connection: " << std::get<int>(task.data)
                  << std::endl;
        break;
      }
      case task_message::TaskType::NEW_TASK: {
        // TODO 新任务

        break;
      }
      default:
        break;
    }
  }
}

void thread_loop(void* args) {
  ThreadQueue<task_message>* queue = (ThreadQueue<task_message>*)args;
  EventLoop* loop = new EventLoop();
  queue->set_loop(loop);
  queue->set_callback(deal_task, queue);
  // loop
  loop->event_process();
}

ThreadPool::ThreadPool(int thread_num) : _thread_num(thread_num), _index(0) {
  if (_thread_num <= 0) {
    _thread_num = 5;
  }
  _queues = std::vector<ThreadQueue<task_message>*>(_thread_num);
  for (int i = 0; i < _thread_num; i++) {
    _queues[i] = new ThreadQueue<task_message>();
    std::thread t(thread_loop, _queues[i]);
    t.detach();
  }
}

ThreadQueue<task_message>* ThreadPool::get_thread_queue() {
  if (_index == _thread_num - 1) {
    _index = 0;
  }
  return _queues[_index++];
}
