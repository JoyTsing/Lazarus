#include <unistd.h>

#include <thread>

#include "dns/dns_client.h"
#include "lars.pb.h"
#include "reporter/report_client.h"
#include "server/udp_server.h"
#include "utils/thread_queue.h"

int main(int argc, const char** argv) {
  // udp-server(需要启动3个)接受业务层-开发者-api层的请求
  start_udp_servers();
  // 启动report-client
  ThreadQueue<lars::ReportStatusRequest> reporter_queue;
  start_report_client();
  // 启动dns-client
  ThreadQueue<lars::GetRouterRequest> dns_queue;
  start_dns_client();
  // 阻塞等待
  std::this_thread::yield();
  pause();
  return 0;
}