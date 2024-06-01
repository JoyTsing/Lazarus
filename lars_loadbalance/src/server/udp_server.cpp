#include "server/udp_server.h"

#include <memory>
#include <thread>

#include "eventloop/event_loop.h"
#include "net/udp/udp_server.h"
#include "utils/minilog.h"

void start_udp_servers() {
  // TODO 可以通过配置文件配置启动几个udp-server
  for (int i = 0; i < 3; i++) {
    std::jthread([i]() {
      // 启动udp-server
      EventLoop loop;
      short port = 8888 + i;
      auto server = std::make_shared<UdpServer>(&loop, "0.0.0.0", port);
      // TODO register router function
      minilog::log_info("LoadBalance agent server:port [{}] is started...",
                        port);
      loop.event_process();
    }).detach();
  }
}