#include <cstdio>
#include <cstring>

#include "eventloop/event_loop.h"
#include "net/net_connection.h"
#include "net/tcp/tcp_client.h"

void handle(const char *data, std::uint32_t len, int msgid, NetConnection *conn,
            void *user_data) {
  // 得到服务端回执的数据
  printf("  callback handler\n");
  printf("  recv from client: %s\n", data);
  printf("  msgid: [%d]\n", msgid);
  printf("  len: [%d]\n", len);
  printf("====================================\n");
}

// 新客户端创建后的回调函数
void on_client_build(NetConnection *conn, void *args) {
  int msgid = 1;
  const char *msg = "hello darkness my old friend";
  conn->send_message(msg, strlen(msg), msgid);
}

// 客户端断开后的回调函数
void on_client_lost(NetConnection *conn, void *args) {
  printf("=====>on client is lost\n");
}

int main() {
  EventLoop loop;
  TCPClient client(&loop, "127.0.0.1", 7777);
  client.add_message_router(1, handle);
  client.add_message_router(200, handle);
  client.set_construct_hook(on_client_build);
  client.set_destruct_hook(on_client_lost);
  loop.event_process();
  return 0;
}