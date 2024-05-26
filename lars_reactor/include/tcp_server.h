#pragma once
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <cstdint>
#include <mutex>
#include <unordered_map>

#include "event_loop.h"
#include "tcp_connection.h"

class TcpServer {
 public:
  TcpServer(EventLoop* loop, const char* ip, std::uint16_t port);
  ~TcpServer();
  void handle_accept();
  // connections
  static void add_connection(int conn_fd, TCPConnection* conn);
  static void remove_connection(int conn_fd);
  static int get_connection_num();

 private:
  int _sockfd;
  struct sockaddr_in _connection_addr;
  socklen_t _connection_addr_len;

  // eventLoop
  EventLoop* _loop;

 private:
  // TODO 从配置文件中读取
#define MAX_CONNS 2
  static std::mutex _mutex;

  static int _max_conns;  // 最大client链接个数
  static std::unordered_map<int, TCPConnection*> _conns;
};