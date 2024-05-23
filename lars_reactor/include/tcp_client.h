#pragma once

#include <netinet/in.h>
#include <unistd.h>

#include "event_loop.h"
#include "reactor_buf.h"

class TCPClient;

using message_call_back =
    std::function<void(const char* data, uint32_t len, int message_id,
                       TCPClient* client, void* user_data)>;

class TCPClient {
 public:
  TCPClient(EventLoop* loop, const char* ip, unsigned short port);

  // send
  int send_message(const char* data, int len, int message_id);

  void clear();

  // set event callback
  void set_callback(message_call_back cb) { _message_cb = cb; }

 private:
  void handle_connect();
  void handle_read();
  void handle_write();
  void handle_connection_delay(int fd);

 private:
  void add_read_event(int fd);
  void add_write_event(int fd);

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