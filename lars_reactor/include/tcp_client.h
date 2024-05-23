#pragma once

#include <netinet/in.h>
#include <unistd.h>

#include "event_loop.h"
#include "reactor_buf.h"

class TcpClient;

using message_call_back =
    std::function<void(const char* data, uint32_t len, int message_id,
                       TcpClient* client, void* user_data)>;

class TcpClient {
 public:
  TcpClient(EventLoop* loop, const char* ip, unsigned short port);

  // send
  int send_message(const char* data, int len, int message_id);

  void clear();

  void handle_connect();

  // set event callback
  void set_message_callback(message_call_back cb) { _message_cb = cb; }

  void handle_read();
  void handle_write();
  void handle_connection_success(int fd);

 private:
  int _sockfd;
  sockaddr_in _server_addr;
  socklen_t _addr_len;
  EventLoop* _loop;
  // handler
  message_call_back _message_cb;
  // buffer
  InputBuffer _input_buf;
  OutputBuffer _output_buf;
};