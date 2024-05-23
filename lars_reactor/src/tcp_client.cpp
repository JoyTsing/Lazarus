#include "tcp_client.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include <iostream>

#include "event_loop.h"
TcpClient::TcpClient(EventLoop* loop, const char* ip, unsigned short port)
    : _loop(loop) {
  _sockfd = -1;
  _message_cb = nullptr;

  bzero(&_server_addr, sizeof(_server_addr));
  _server_addr.sin_family = AF_INET;
  inet_aton(ip, &_server_addr.sin_addr);
  _server_addr.sin_port = htons(port);
  _addr_len = sizeof(_server_addr);
  this->handle_connect();
}

int TcpClient::send_message(const char* data, int len, int message_id) {
  std::cout << "TcpClient::send_message\n";
  return 0;
}

void TcpClient::clear() {
  if (_sockfd != -1) {
    _loop->del_io_event(_sockfd);
    close(_sockfd);
  }
  _sockfd = -1;
  // 长连接则重新连接
}

void TcpClient::handle_read() { std::cout << "TcpClient::handle_read\n"; }

void TcpClient::handle_write() { std::cout << "TcpClient::handle_write\n"; }

void TcpClient::handle_connection_success(int fd) {
  std::cout << "TcpClient::handle_connection_success\n";
  _loop->del_io_event(fd);
}

void TcpClient::handle_connect() {
  if (_sockfd != -1) {
    close(_sockfd);
  }
  _sockfd =
      socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, IPPROTO_TCP);
  if (_sockfd == -1) {
    // handle error
    std::cerr << "client create socket error\n";
    exit(1);
  }
  int ret = connect(_sockfd, (const struct sockaddr*)&_server_addr, _addr_len);
  if (ret == -1) {
    if (errno == EINPROGRESS) {
      // connect in progress
      // check writable
      _loop->add_io_event(
          _sockfd, EPOLLOUT,
          [](IO_EVENT_ARGUMENT) {
            TcpClient* client = (TcpClient*)args;
            client->handle_connection_success(fd);
          },
          this);
    } else {
      // handle error
      std::cerr << "client connect error\n";
      exit(1);
    }
  }
  _loop->add_io_event(
      _sockfd, EPOLLIN,
      [](IO_EVENT_ARGUMENT) {
        TcpClient* client = (TcpClient*)args;
        client->handle_read();
      },
      this);
}
