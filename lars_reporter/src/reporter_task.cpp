#include "reporter_task.h"

#include <memory>
#include <queue>
#include <thread>
#include <vector>

#include "eventloop/event_loop.h"
#include "lars.pb.h"
#include "reporter_store.h"
#include "utils/config_file.h"
#include "utils/thread_queue.h"

static int thread_db_num = 0;
std::vector<std::shared_ptr<ThreadQueue<lars::ReportStatusRequest>>>
    reporter_queues;
std::vector<std::shared_ptr<ReporterStore>> reporter_stores;

void get_reporter_status(MESSAGE_ROUTER_ARGS) {
  // round robin
  static int cur = 0;
  lars::ReportStatusRequest request;
  request.ParseFromArray(data, len);
  // 将请求给threadpool处理
  reporter_queues[cur++]->send(request);
  cur = cur % thread_db_num;
}

void create_store_threadpool() {
  thread_db_num =
      config_file::instance()->GetNumber("reporter", "db_threadnum", 3);
  reporter_queues.reserve(thread_db_num);
  for (int i = 0; i < thread_db_num; i++) {
    reporter_queues.push_back(
        std::make_shared<ThreadQueue<lars::ReportStatusRequest>>());
    reporter_stores.push_back(std::make_shared<ReporterStore>());
    std::jthread(
        [](int i) {
          // 处理磁盘IO
          store_handler(i);
        },
        i)
        .detach();
  }
}

void store_handler(int index) {
  EventLoop loop;
  auto queue = reporter_queues[index];
  queue->set_loop(&loop);
  // 设置磁盘处理的回调函数
  queue->set_callback(
      [](EventLoop* loop, int fd, void* args) {
        int index = *static_cast<int*>(args);
        auto queue = reporter_queues[index];
        auto store = reporter_stores[index];
        std::queue<lars::ReportStatusRequest> report_message;
        queue->recv(report_message);
        while (!report_message.empty()) {
          lars::ReportStatusRequest message = report_message.front();
          report_message.pop();
          // 处理磁盘IO
          store->store(message);
        }
      },
      static_cast<void*>(&index));
  loop.event_process();
}