#include <cstdio>

#include "event_loop.h"
#include "net_connection.h"
#include "tcp_client.h"

void handle(const char *data, std::uint32_t len, int msgid, NetConnection *conn,
            void *user_data) {
  // 得到服务端回执的数据
  printf("  callback handler\n");
  printf("  recv from client: %s\n", data);
  printf("  msgid: [%d]\n", msgid);
  printf("  len: [%d]\n", len);
  printf("====================================\n");
}

int main() {
  EventLoop loop;
  TCPClient client(&loop, "127.0.0.1", 7777);
  client.add_message_router(1, handle);
  loop.event_process();
  return 0;
}