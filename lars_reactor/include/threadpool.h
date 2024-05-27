#pragma once

#include <vector>

#include "task_message.h"
#include "thread_queue.h"

class ThreadPool {
 public:
  ThreadPool(int thread_num = 5);

 private:
  int _thread_num;
  std::vector<ThreadQueue<task_message>*> _queues;
};