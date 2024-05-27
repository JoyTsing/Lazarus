#pragma once
#include <netinet/in.h>

#include <cstdint>

#include "eventloop/event_loop.h"
#include "message/message.h"
#include "net/net_connection.h"

class UdpServer : public NetConnection {
 public:
  UdpServer(EventLoop* loop, const char* ip, std::uint16_t port);
  ~UdpServer();
  int send_message(const char* data, int len, int message_id) override;

  // add message router
  void add_message_router(int msg_id, message_callback handler,
                          void* args = nullptr);

 private:
  void handle_read();

  void add_read_event(int fd);

 private:
  int _sockfd;
  struct sockaddr_in _connection_addr;
  socklen_t _connection_addr_len;

  // eventLoop
  EventLoop* _loop;
  // router handler
  message_router _router;
  // buffer
  char _read_buffer[MESSAGE_LENGTH_LIMIT];
  char _write_buffer[MESSAGE_LENGTH_LIMIT];
};