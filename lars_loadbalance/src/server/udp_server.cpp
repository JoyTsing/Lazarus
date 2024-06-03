#include "server/udp_server.h"

#include <memory>
#include <string>
#include <thread>

#include "base/init.h"
#include "eventloop/event_loop.h"
#include "lars.pb.h"
#include "net/udp/udp_server.h"
#include "utils/minilog.h"

void loadbalance::start_udp_servers() {
  // TODO 可以通过配置文件配置启动几个udp-server
  for (int i = 0; i < 3; i++) {
    std::jthread(
        [](int i) {
          // 启动udp-server
          EventLoop loop;
          short port = 8888 + i;
          auto server = std::make_shared<UdpServer>(&loop, "0.0.0.0", port);
          // register router function
          server->add_message_router(lars::ID_GetHostRequest,
                                     server::handle_get_host, &i);
          server->add_message_router(lars::ID_ReportRequest,
                                     server::handle_get_report, &i);
          minilog::log_info("LoadBalance agent server:port [{}] is started...",
                            port);
          loop.event_process();
        },
        i)
        .detach();
  }
}

void loadbalance::server::handle_get_host(MESSAGE_ROUTER_ARGS) {
  // 解析消息
  // minilog::log_info("get host request from client");
  lars::GetHostRequest request;
  request.ParseFromArray(data, len);
  int modid = request.modid();
  int cmdid = request.cmdid();
  // reply
  lars::GetHostResponse response;
  response.set_seq(request.seq());
  response.set_modid(modid);
  response.set_cmdid(cmdid);
  // 通过route_balance 获取host 并填充到response
  int index = *static_cast<int*>(user_data);
  auto router = base::route_balances[index];
  router->get_host(modid, cmdid, response);
  // 将response发回给客户端
  std::string response_str;
  response.SerializeToString(&response_str);
  client->send_message(response_str.c_str(), response_str.size(),
                       lars::ID_GetHostResponse);
}

void loadbalance::server::handle_get_report(MESSAGE_ROUTER_ARGS) {
  minilog::log_info("report request from client");
  lars::ReportRequest request;
  request.ParseFromArray(data, len);
  int index = *static_cast<int*>(user_data);
  auto router = base::route_balances[index];
  router->report(request);
}