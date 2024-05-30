#pragma once

#include <netinet/in.h>
#include <unistd.h>

#include "buffer/reactor_buf.h"
#include "eventloop/event_loop.h"
#include "message/message.h"
#include "net/net_connection.h"

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

  // set con/des hook function
  void set_construct_hook(connection_callback hook, void* args = nullptr) {
    _construct_hook = hook;
    _construct_hook_args = args;
  }

  void set_destruct_hook(connection_callback hook, void* args = nullptr) {
    _destruct_hook = hook;
    _destruct_hook_args = args;
  }

  int get_fd() override { return _sockfd; }

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
  // hook function
  connection_callback _construct_hook;
  void* _construct_hook_args;

  connection_callback _destruct_hook;
  void* _destruct_hook_args;
};