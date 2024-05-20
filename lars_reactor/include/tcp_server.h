#pragma once
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <cstdint>

class TcpServer {
 public:
  TcpServer(const char* ip, std::uint16_t port);
  ~TcpServer();
  void do_accept();

 private:
  int _sockfd;
  struct sockaddr_in _connection_addr;
  socklen_t _connection_addr_len;
};