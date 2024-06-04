#include <cstring>
#include <memory>
#include <thread>

#include "dns_logo.h"
#include "dns_subscribe.h"
#include "dns_task.h"
#include "eventloop/event_loop.h"
#include "lars.pb.h"
#include "net/net_connection.h"
#include "net/tcp/tcp_server.h"
#include "utils/config_file.h"
#include "utils/minilog.h"

std::unique_ptr<TcpServer> server;

// 客户端链接成功后执行
void create_subscribe(NetConnection* conn, void* args) {
  // 给conn绑定订阅的mod的集合
  conn->param = new client_subscribe_set();
}

// 客户端断开链接后执行
void clear_subscribe(NetConnection* conn, void* args) {
  // 释放绑定的mod集合
  client_subscribe_set* set = (client_subscribe_set*)conn->param;
  for (auto mod : *set) {
    int modid = (int)(mod >> 32);
    int cmdid = (int)mod;
    minilog::log_info(" @fd [{}] unsubscribe: modid= {}, cmdid={}",
                      conn->get_fd(), modid, cmdid);
    SubscribeList::instance()->unsubscribe(mod, conn->get_fd());
  }
  delete set;
  conn->param = nullptr;
}

int main(int argc, const char** argv) {
  dns_logo();
  EventLoop loop;
  config_file::setPath("./conf/lars_dns.conf");
  std::string ip =
      config_file::instance()->GetString("reactor", "ip", "0.0.0.0");
  short port = config_file::instance()->GetNumber("reactor", "port", 7777);

  server = std::make_unique<TcpServer>(&loop, ip.c_str(), port);
  // cons/dest hook
  server->set_conn_start_hook(create_subscribe);
  server->set_conn_close_hook(clear_subscribe);
  // 添加回调
  server->add_message_router(lars::ID_GetRouterRequest, get_router);

  // backend thread监控版本信息
  std::jthread(check_router_change, nullptr).detach();

  loop.event_process();
  return 0;
}