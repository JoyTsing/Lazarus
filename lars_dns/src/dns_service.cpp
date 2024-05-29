#include <cstring>

#include "dns_router.h"
#include "eventloop/event_loop.h"
#include "lars.pb.h"
#include "net/tcp/tcp_server.h"
#include "utils/config_file.h"
#include "utils/minilog.h"

TcpServer* server;

// 获取路由信息
void get_router(const char* data, std::uint32_t len, int message_id,
                NetConnection* conn, void* user_data) {
  // 1. 解析proto
  lars::GetRouterRequest request;
  request.ParseFromArray(data, len);
  // 2. get modID & cmdID
  int modid, cmdid;
  modid = request.modid();
  cmdid = request.cmdid();
  // 3. parse to host
  host_set hosts = Router::instance()->get_hosts(modid, cmdid);
  // 4. response
  lars::GetRouterResponse response;
  response.set_modid(modid);
  response.set_cmdid(cmdid);
  for (auto host : hosts) {
    lars::HostInfo host_info;
    host_info.set_ip((std::uint32_t)(host >> 32));
    host_info.set_port((int)host);
    // add to response
    response.add_hosts()->CopyFrom(host_info);
    // minilog::log_info("host ip: {}, port: {}", (std::uint32_t)host_info.ip(),
    //                   host_info.port());
  }
  // 5. send
  std::string send_data;
  response.SerializeToString(&send_data);
  conn->send_message(send_data.c_str(), send_data.size(),
                     lars::ID_GetRouterResponse);
}

int main(int argc, const char** argv) {
  EventLoop loop;
  config_file::setPath("./conf/lars_dns.conf");
  std::string ip =
      config_file::instance()->GetString("reactor", "ip", "0.0.0.0");
  short port = config_file::instance()->GetNumber("reactor", "port", 7777);

  server = new TcpServer(&loop, ip.c_str(), port);
  // 添加回调
  server->add_message_router(lars::ID_GetRouterRequest, get_router);
  Router::instance();

  loop.event_process();
  return 0;
}