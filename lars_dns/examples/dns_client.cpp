
#include <string>

#include "eventloop/event_loop.h"
#include "lars.pb.h"
#include "net/net_connection.h"
#include "net/tcp/tcp_client.h"
#include "utils/minilog.h"

// 连接建立后请求host信息
void on_connection(NetConnection* conn, void* args) {
  lars::GetRouterRequest request;
  request.set_modid(1);
  request.set_cmdid(1);
  std::string request_str;

  // 序列化
  request.SerializeToString(&request_str);
  conn->send_message(request_str.c_str(), request_str.size(),
                     lars::ID_GetRouterRequest);
}

// 处理dns回复的消息
void handle_router(MESSAGE_ROUTER_ARGS) {
  lars::GetRouterResponse response;
  response.ParseFromArray(data, len);
  // 打印
  minilog::log_info("modid = {}, cmd id ={}, host_size= {}", response.modid(),
                    response.cmdid(), response.hosts_size());
  for (int i = 0; i < response.hosts_size(); i++) {
    minilog::log_info(" @mods[{}]==> ip = {}, port = {}", i,
                      (std::uint32_t)response.hosts(i).ip(),
                      response.hosts(i).port());
  }
}

int main(int argc, const char** argv) {
  EventLoop loop;
  TCPClient* client = new TCPClient(&loop, "127.0.0.1", 7778);
  client->set_construct_hook(on_connection);
  client->add_message_router(lars::ID_GetRouterResponse, handle_router);
  loop.event_process();
  return 0;
}