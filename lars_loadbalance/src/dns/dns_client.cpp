#include "dns/dns_client.h"

#include <queue>
#include <thread>

#include "base/init.h"
#include "eventloop/event_base.h"
#include "eventloop/event_loop.h"
#include "lars.pb.h"
#include "net/tcp/tcp_client.h"
#include "utils/config_file.h"
#include "utils/minilog.h"

void loadbalance::start_dns_client() {
  std::jthread([]() {
    minilog::log_info("dns client start...");
    loadbalance::dns::handle_dns_client();
  }).detach();
}

void loadbalance::dns::handle_dns_read(IO_EVENT_ARGUMENT) {
  std::queue<lars::GetRouterRequest> messages;
  TCPClient *client = (TCPClient *)args;
  loadbalance::base::dns_queue->recv(messages);
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
  // 让loop监控queue的数据
  loadbalance::base::dns_queue->set_loop(&loop);
  loadbalance::base::dns_queue->set_callback(handle_dns_read, &client);
  loop.event_process();
}
