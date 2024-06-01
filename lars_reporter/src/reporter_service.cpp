#include <cstring>
#include <memory>

#include "eventloop/event_loop.h"
#include "lars.pb.h"
#include "net/tcp/tcp_server.h"
#include "reporter_task.h"
#include "utils/config_file.h"
#include "utils/minilog.h"

std::unique_ptr<TcpServer> server;

int main(int argc, const char** argv) {
  EventLoop loop;
  config_file::setPath("./conf/lars_reporter.conf");
  std::string ip =
      config_file::instance()->GetString("reactor", "ip", "0.0.0.0");
  short port = config_file::instance()->GetNumber("reactor", "port", 7777);

  server = std::make_unique<TcpServer>(&loop, ip.c_str(), port);
  // 添加上报请求的处理消息分发业务
  server->add_message_router(lars::ID_ReportStatusRequest, get_reporter_status);
  // cons/dest hook

  loop.event_process();
  return 0;
}