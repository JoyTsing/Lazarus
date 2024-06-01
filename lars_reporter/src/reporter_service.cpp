#include <cstring>
#include <memory>

#include "eventloop/event_loop.h"
#include "net/tcp/tcp_server.h"
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
  // cons/dest hook

  // 添加回调

  loop.event_process();
  return 0;
}