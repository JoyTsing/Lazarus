#include "reporter/report_client.h"

#include <thread>

#include "utils/minilog.h"

void start_report_client() {
  // 启动一个线程，不断的从队列中取出数据，发送给reporter-server
  std::jthread([]() { handle_report(); }).detach();
}

void handle_report() { minilog::log_info("report client start..."); }