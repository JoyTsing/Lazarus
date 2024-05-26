#pragma once

#include <netinet/in.h>
#include <unistd.h>

#include "event_loop.h"
#include "message.h"
#include "net_connection.h"
#include "reactor_buf.h"

class TCPClient : public NetConnection {
 public:
  TCPClient(EventLoop* loop, const char* ip, unsigned short port);

  // send
  int send_message(const char* data, int len, int message_id) override;

  void clear();

  // set event callback
  void set_callback(message_callback cb) { this->_message_cb = cb; }

 private:
  void handle_connect();
  void handle_read();
  void handle_write();
  void handle_connection_delay();

 private:
  void add_read_event(int fd);
  void add_write_event(int fd);

 private:
  int _sockfd;
  sockaddr_in _server_addr;
  socklen_t _addr_len;
  EventLoop* _loop;
  // handler
  message_callback _message_cb;
  // buffer
  InputBuffer _input_buf;
  OutputBuffer _output_buf;
};