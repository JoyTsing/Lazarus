#include "server/udp_server.h"

#include <memory>
#include <thread>

#include "eventloop/event_loop.h"
#include "lars.pb.h"
#include "net/udp/udp_server.h"
#include "utils/minilog.h"

void loadbalance::start_udp_servers() {
  // TODO 可以通过配置文件配置启动几个udp-server
  for (int i = 0; i < 3; i++) {
    std::jthread([i]() {
      // 启动udp-server
      EventLoop loop;
      short port = 8888 + i;
      auto server = std::make_shared<UdpServer>(&loop, "0.0.0.0", port);
      // register router function
      server->add_message_router(lars::ID_GetHostRequest,
                                 loadbalance::server::handle_get_host);
      minilog::log_info("LoadBalance agent server:port [{}] is started...",
                        port);
      loop.event_process();
    }).detach();
  }
}

// 不污染全局命名空间
void loadbalance::server::handle_get_host(MESSAGE_ROUTER_ARGS) {}
