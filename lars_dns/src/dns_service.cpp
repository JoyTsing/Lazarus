#include <cstdint>
#include <cstring>
#include <memory>
#include <thread>
#include <vector>

#include "dns_router.h"
#include "dns_subscribe.h"
#include "eventloop/event_loop.h"
#include "lars.pb.h"
#include "net/net_connection.h"
#include "net/tcp/tcp_server.h"
#include "utils/config_file.h"
#include "utils/minilog.h"

TcpServer* server;
using client_subscribe_set = host_set;
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
  // 2.5 订阅
  std::uint64_t mod = ((std::uint64_t)modid << 32) + cmdid;
  client_subscribe_set* set = (client_subscribe_set*)conn->param;
  if (set == nullptr) {
    minilog::log_error("subscribe set is nullptr");
  } else if (set->find(mod) == set->end()) {
    SubscribeList::instance()->subscribe(mod, conn->get_fd());
    set->insert(mod);
    minilog::log_info("fd {} subscribe modid = {}, cmdid= {}", conn->get_fd(),
                      modid, cmdid);
  }
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
    SubscribeList::instance()->unsubscribe(mod, conn->get_fd());
  }
  delete set;
  conn->param = nullptr;
}

int main(int argc, const char** argv) {
  EventLoop loop;
  config_file::setPath("./conf/lars_dns.conf");
  std::string ip =
      config_file::instance()->GetString("reactor", "ip", "0.0.0.0");
  short port = config_file::instance()->GetNumber("reactor", "port", 7777);

  server = new TcpServer(&loop, ip.c_str(), port);
  // cons/dest hook
  server->set_construct_hook(create_subscribe);
  server->set_destruct_hook(clear_subscribe);
  // 添加回调
  server->add_message_router(lars::ID_GetRouterRequest, get_router);

  // 定期发布更变mod的集合
  std::jthread([]() {
    while (true) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      // modid =1,cmdid=1
      int modid = 1, cmdid = 1;
      std::uint64_t mod = ((std::uint64_t)modid << 32) + cmdid;
      // modid =1,cmdid=2
      int modid2 = 1, cmdid2 = 2;
      std::uint64_t mod2 = ((std::uint64_t)modid2 << 32) + cmdid2;
      std::vector<std::uint64_t> change_mods = {mod, mod2};
      SubscribeList::instance()->publish(change_mods);
    }
  }).detach();

  loop.event_process();
  return 0;
}