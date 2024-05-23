#include "tcp_client.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include <cstring>
#include <iostream>

#include "event_loop.h"
#include "message.h"
TCPClient::TCPClient(EventLoop* loop, const char* ip, unsigned short port)
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

int TCPClient::send_message(const char* data, int len, int message_id) {
  std::cout << "TcpClient::send_message\n";
  bool active_epollout = false;
  if (_output_buf.length() == 0) {
    active_epollout = true;
  }
  // head
  message_head head;
  head.message_id = message_id;
  head.message_len = len;
  // add head to output buffer
  int ret = _output_buf.add_data((const char*)&head, MESSAGE_HEAD_LEN);
  if (ret == -1) {
    return -1;
  }
  ret = _output_buf.add_data(data, len);
  if (ret == -1) {
    _output_buf.pop(MESSAGE_HEAD_LEN);
    return -1;
  }
  if (active_epollout) {
    add_write_event(_sockfd);
  }
  return 0;
}

void TCPClient::clear() {
  if (_sockfd != -1) {
    _loop->del_io_event(_sockfd);
    close(_sockfd);
  }
  _sockfd = -1;
  // 长连接则重新连接
}

void TCPClient::handle_read() {
  std::cout << "TcpClient::handle_read\n";
  // 1 read data from connection fd
  int ret = _input_buf.read_data(_sockfd);
  if (ret == -1 || ret == 0) {
    clear();
    return;
  }
  // 2 check if data is ready to read aka. at least 8 bytes(message header)
  message_head head;
  while (_input_buf.length() >= MESSAGE_HEAD_LEN) {
    // 2.1 parse message header
    memcpy(&head, _input_buf.data(), MESSAGE_HEAD_LEN);
    if (head.message_len > MESSAGE_LENGTH_LIMIT || head.message_len < 0) {
      clear();
      break;
    }
    // 2.2 check valid
    if (_input_buf.length() < MESSAGE_HEAD_LEN + head.message_len) {
      break;
    }
    // 3 handle message
    _input_buf.pop(MESSAGE_HEAD_LEN);
    if (_message_cb) {
      _message_cb(_input_buf.data(), head.message_len, head.message_id, nullptr,
                  this);
    }
    _input_buf.pop(head.message_len);
  }
  _input_buf.adjust();
}

void TCPClient::handle_write() {  // 此时output buffer中有数据
  std::cout << "TcpClient::handle_write\n";
  while (_output_buf.length() > 0) {
    int ret = _output_buf.write_to(_sockfd);
    if (ret == -1) {
      clear();
      return;
    } else if (ret == 0) {
      break;
    }
  }
  if (_output_buf.length() == 0) {
    add_read_event(_sockfd);  // TODO should be remove
  }
}

void TCPClient::handle_connection_delay(int fd) {
  std::cout << "TcpClient::handle_connection_success\n";
  _loop->del_io_event(fd);
  // check
  int res = 0;
  socklen_t len = sizeof(res);
  getsockopt(fd, SOL_SOCKET, SO_ERROR, &res, &len);
  if (res != 0) {
    // handle error
    std::cerr << "client connect error\n";
    return;
  }
  std::cout << "connect success\n";
  // handle
  const char* message = "hello from client\n";
  int msgid = 1;
  send_message(message, strlen(message), msgid);
  // add read event
  add_read_event(fd);
  if (_output_buf.length() != 0) {
    add_write_event(fd);
  }
}

void TCPClient::add_read_event(int fd) {
  _loop->add_io_event(
      fd, EPOLLIN,
      [](IO_EVENT_ARGUMENT) {
        TCPClient* client = (TCPClient*)args;
        client->handle_read();
      },
      this);
}

void TCPClient::add_write_event(int fd) {
  _loop->add_io_event(
      fd, EPOLLOUT,
      [](IO_EVENT_ARGUMENT) {
        TCPClient* client = (TCPClient*)args;
        client->handle_write();
      },
      this);
}

void TCPClient::handle_connect() {
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
      // fd is non-blocking, connect is in progress
      fprintf(stderr, "do_connect EINPROGRESS\n");
      _loop->add_io_event(
          _sockfd, EPOLLOUT,
          [](IO_EVENT_ARGUMENT) {
            TCPClient* client = (TCPClient*)args;
            client->handle_connection_delay(fd);
          },
          this);
      return;
    } else {
      // handle error
      std::cerr << "client connect error\n";
      exit(1);
    }
  }
  handle_connection_delay(_sockfd);
}
