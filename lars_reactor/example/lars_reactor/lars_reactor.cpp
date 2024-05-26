#include "tcp_server.h"

void handle(const char *data, std::uint32_t len, int msgid, NetConnection *conn,
            void *user_data) {
  // 得到服务端回执的数据
  printf("server callback\n");
  // 发送数据给服务端
  conn->send_message(data, len, msgid);
}

void handle2(const char *data, std::uint32_t len, int msgid,
             NetConnection *conn, void *user_data) {
  // 得到服务端回执的数据
  printf("  server callback handler2\n");
  printf("  recv from client: %s\n", data);
  printf("  msgid: [%d]\n", msgid);
  printf("  len: [%d]\n", len);
  printf("====================================\n");
}

int main(int argc, const char **argv) {
  EventLoop loop;
  TcpServer server(&loop, "127.0.0.1", 7777);
  server.add_message_router(1, handle);
  server.add_message_router(2, handle2);
  loop.event_process();
  return 0;
}