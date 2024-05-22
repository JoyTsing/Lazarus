#include "tcp_server.h"

int main(int argc, const char** argv) {
  EventLoop loop;
  TcpServer server(&loop, "127.0.0.1", 7777);
  loop.event_process();

  return 0;
}