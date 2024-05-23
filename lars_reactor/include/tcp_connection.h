#pragma once

#include "event_loop.h"
#include "reactor_buf.h"

// using io_call_back = std::function<void(EventLoop* el, int fd, void* args)>;

class TCPConnection {
 public:
  TCPConnection(int conn_fd, EventLoop* event_loop);
  // clear
  void clear();

  int send_message(const char* data, int len, int message_id);

 private:
  /**
   * @brief read event , and cut the message package
   *
   */
  void handle_read();
  // write event
  void handle_write();
  // patch message

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