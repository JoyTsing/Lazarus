#include <cstdio>

#include "event_loop.h"
#include "net_connection.h"
#include "tcp_client.h"
// 客户端业务
void handle(const char *data, std::uint32_t len, int msgid, NetConnection *conn,
            void *user_data) {
  // 得到服务端回执的数据
  printf("recv server: [%s]\n", data);
  printf("msgid: [%d]\n", msgid);
  printf("len: [%d]\n", len);
  printf("====================================\n");
  // 发送数据给服务端
  // conn->send_message(data, len, msgid + 1);
}

int main() {
  EventLoop loop;
  TCPClient client(&loop, "127.0.0.1", 7777);
  client.set_callback(handle);
  loop.event_process();
  return 0;
}