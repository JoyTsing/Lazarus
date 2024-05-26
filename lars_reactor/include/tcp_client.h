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

  // register router
  void add_message_router(int msg_id, message_callback handler,
                          void* args = nullptr) {
    _router.register_router(msg_id, handler, args);
  }

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
  // buffer
  InputBuffer _input_buf;
  OutputBuffer _output_buf;
  // router
  message_router _router;
};