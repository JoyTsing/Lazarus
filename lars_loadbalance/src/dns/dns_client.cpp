#include "dns/dns_client.h"

#include <queue>
#include <thread>

#include "base/init.h"
#include "eventloop/event_base.h"
#include "eventloop/event_loop.h"
#include "lars.pb.h"
#include "net/tcp/tcp_client.h"
#include "utils/config_file.h"
#include "utils/hash.h"
#include "utils/minilog.h"

void loadbalance::start_dns_client() {
  std::jthread([]() {
    minilog::log_info("dns client start...");
    loadbalance::dns::handle_dns_client();
  }).detach();
}

// 用于处理dns server回复当前modid/cmdid对应的所有host信息
void loadbalance::dns::handle_route_recv(MESSAGE_ROUTER_ARGS) {
  lars::GetRouterResponse response;
  response.ParseFromArray(data, len);
  // 1. modid和cmdid
  int modid = response.modid();
  int cmdid = response.cmdid();
  // 2. 获取对应的load balance
  // 获取udp server id
  int index = hash_index(modid, cmdid);
  // 3. 获取对应的load balance
  base::route_balances[index]->update_host(modid, cmdid, response);
}

void loadbalance::dns::handle_dns_request(IO_EVENT_ARGUMENT) {
  std::queue<lars::GetRouterRequest> messages;
  TCPClient *client = (TCPClient *)args;
  base::dns_queue->recv(messages);
  while (!messages.empty()) {
    lars::GetRouterRequest request = messages.front();
    messages.pop();

    std::string request_string;
    request.SerializeToString(&request_string);

    // 发送dns-server
    client->send_message(request_string.c_str(), request_string.size(),
                         lars::ID_GetRouterRequest);
  }
}

void loadbalance::dns::handle_dns_client() {
  EventLoop loop;
  // parser config
  std::string ip = config_file::instance()->GetString("dns", "ip", "127.0.0.1");
  short port = config_file::instance()->GetNumber("dns", "port", 9999);
  // client
  TCPClient client(&loop, ip.c_str(), port);
  // 消息队列与事件绑定
  base::dns_queue->set_loop(&loop);
  base::dns_queue->set_callback(handle_dns_request, &client);
  // 用于处理dns server的回复
  client.add_message_router(lars::ID_GetRouterResponse, handle_route_recv);
  loop.event_process();
}
