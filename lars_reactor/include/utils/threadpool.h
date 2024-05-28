#pragma once

#include <vector>

#include "message/task_message.h"
#include "utils/thread_queue.h"

class ThreadPool {
 public:
  ThreadPool(int thread_num = 5);

  /**
   * @brief Get the thread queue object by round robin
   *
   * @return ThreadQueue<task_message>*
   */
  ThreadQueue<task_message>* get_thread_queue();

  /**
   * @brief 发送一个NEW_TASK类型的task到线程池
   *
   * @param func
   * @param args
   */
  void send_task(task_func func, void* args = nullptr);

 private:
  int _thread_num;
  int _index;
  std::vector<ThreadQueue<task_message>*> _queues;
};
