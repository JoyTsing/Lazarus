#pragma once
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <cstdint>

#include "event_loop.h"

class TcpServer {
 public:
  TcpServer(EventLoop* loop, const char* ip, std::uint16_t port);
  ~TcpServer();
  void do_accept();

 private:
  int _sockfd;
  struct sockaddr_in _connection_addr;
  socklen_t _connection_addr_len;

  // eventLoop
  EventLoop* _loop;
};