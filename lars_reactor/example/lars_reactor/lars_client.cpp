#include "event_loop.h"
#include "tcp_client.h"

int main() {
  EventLoop loop;
  TcpClient client(&loop, "127.0.0.1", 7777);
  client.handle_connect();
  return 0;
}