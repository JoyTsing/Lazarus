#pragma once

#include "buffer/reactor_buf.h"
#include "eventloop/event_loop.h"
#include "net/net_connection.h"

// using io_call_back = std::function<void(EventLoop* el, int fd, void* args)>;

class TCPConnection : public NetConnection {
 public:
  TCPConnection(int conn_fd, EventLoop* event_loop);
  ~TCPConnection();
  // clear
  void clear();

  int send_message(const char* data, int len, int message_id) override;

 private:
  /**
   * @brief read event , and cut the message package
   *
   */
  void handle_read();
  // write event
  void handle_write();

 private:
  void add_read_event(int fd);
  void add_write_event(int fd);

 private:
  // fd
  int _conn_fd;
  // based on which event loop
  EventLoop* _event_loop;
  // output buffer
  OutputBuffer _output_buf;
  // input buffer
  InputBuffer _input_buf;
};