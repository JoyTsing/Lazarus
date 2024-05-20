#include "tcp_server.h"

int main(int argc, const char** argv) {
  TcpServer server("127.0.0.1", 7777);
  server.do_accept();

  return 0;
}