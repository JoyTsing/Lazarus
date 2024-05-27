#include <cstdio>
#include <cstring>

#include "event_loop.h"
#include "net_connection.h"
#include "udp_client.h"

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
  UdpClient client(&loop, "127.0.0.1", 7777);
  client.add_message_router(1, handle);
  int msgid = 1;
  const char *data = "hello udp server";
  client.send_message(data, strlen(data), msgid);
  loop.event_process();
  return 0;
}