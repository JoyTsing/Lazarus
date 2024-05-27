#pragma once

#include <netinet/in.h>

#include "event_loop.h"
#include "message.h"
#include "net_connection.h"

class UdpClient : public NetConnection {
 public:
  UdpClient(EventLoop* loop, const char* ip, std::uint16_t port);
  ~UdpClient();

  int send_message(const char* data, int len, int message_id) override;
  // add message router
  void add_message_router(int msg_id, message_callback handler,
                          void* args = nullptr);

 private:
  void handle_read();

  void add_read_event(int fd);

 private:
  int _sockfd;
  // eventLoop
  EventLoop* _loop;
  // buffer
  char _read_buffer[MESSAGE_LENGTH_LIMIT];
  char _write_buffer[MESSAGE_LENGTH_LIMIT];
  // router handler
  message_router _router;
};