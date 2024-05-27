#include "net/tcp/tcp_connection.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#include "buffer/reactor_buf.h"
#include "eventloop/event_loop.h"
#include "message/message.h"
#include "net/tcp/tcp_server.h"

void handle_test(const char* data, uint32_t len, int msgid, void* args,
                 TCPConnection* conn) {
  conn->send_message(data, len, msgid);
}

TCPConnection::~TCPConnection() { clear(); }

void TCPConnection::add_read_event(int fd) {
  _event_loop->add_io_event(
      fd, EPOLLIN,
      [](IO_EVENT_ARGUMENT) {
        TCPConnection* conn = (TCPConnection*)args;
        conn->handle_read();
      },
      this);
}

void TCPConnection::add_write_event(int fd) {
  _event_loop->add_io_event(
      fd, EPOLLOUT,
      [](IO_EVENT_ARGUMENT) {
        TCPConnection* conn = (TCPConnection*)args;
        conn->handle_write();
      },
      this);
}

TCPConnection::TCPConnection(int conn_fd, EventLoop* event_loop)
    : _conn_fd(conn_fd), _event_loop(event_loop) {
  // connection fd set non-blocking
  int flags = fcntl(_conn_fd, F_GETFL, 0);
  fcntl(_conn_fd, F_SETFL, flags | O_NONBLOCK);
  // TCP_NODELAY
  int op = 1;
  setsockopt(_conn_fd, IPPROTO_TCP, TCP_NODELAY, &op, sizeof(op));
  // hook function
  if (TcpServer::_construct_hook != nullptr) {
    TcpServer::_construct_hook(this, TcpServer::_construct_hook_args);
  }

  // use lambda function to bind read and write event
  add_read_event(_conn_fd);
  // add self to tcp_server connections
  TcpServer::add_connection(_conn_fd, this);
}

void TCPConnection::handle_read() {
  // 1 read data from connection fd
  int ret = _input_buf.read_data(_conn_fd);
  if (ret == -1) {
    clear();
    return;
  } else if (ret == 0) {
    std::cerr << "connection closed by peer\n";
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
    TcpServer::call_router(head.message_id, head.message_len, _input_buf.data(),
                           this);
    _input_buf.pop(head.message_len);
  }
  _input_buf.adjust();
}

void TCPConnection::handle_write() {
  // 此时output buffer中有数据
  while (_output_buf.length()) {
    int ret = _output_buf.write_to(_conn_fd);
    if (ret == -1) {
      clear();
      return;
    }
    if (ret == 0) {
      break;
    }
  }
  if (_output_buf.length() == 0) {
    _event_loop->del_io_event(_conn_fd, EPOLLOUT);
  }
}

int TCPConnection::send_message(const char* data, int len, int message_id) {
  bool epollout = false;
  if (_output_buf.length() == 0) {
    // when output buffer is empty, we need to register write event
    epollout = true;
  }
  // head
  message_head head;
  head.message_id = message_id;
  head.message_len = len;
  int ret = _output_buf.add_data((const char*)&head, MESSAGE_HEAD_LEN);
  if (ret != 0) {
    std::cerr << "send head error\n";
    return -1;
  }
  // data
  ret = _output_buf.add_data(data, len);
  if (ret != 0) {
    _output_buf.pop(MESSAGE_HEAD_LEN);
    std::cerr << "send head error\n";
    return -1;
  }
  // register write event
  if (epollout) {
    add_write_event(_conn_fd);
  }
  return 0;
}

void TCPConnection::clear() {
  // destruction hook function
  if (TcpServer::_destruct_hook != nullptr) {
    TcpServer::_destruct_hook(this, TcpServer::_destruct_hook_args);
  }
  // remove from tcp_server connections
  TcpServer::remove_connection(_conn_fd);
  _event_loop->del_io_event(_conn_fd);

  _input_buf.clear();
  _output_buf.clear();
  close(_conn_fd);
  _conn_fd = -1;
}
