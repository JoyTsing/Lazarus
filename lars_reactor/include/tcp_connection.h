#pragma once

#include "event_loop.h"
#include "reactor_buf.h"

// using io_call_back = std::function<void(EventLoop* el, int fd, void* args)>;

class TCPConnection {
 public:
  TCPConnection(int conn_fd, EventLoop* event_loop);
  /**
   * @brief read event , and cut the message package
   *
   */
  void handle_read(IO_EVENT_ARGUMENT);
  // write event
  void handle_write(IO_EVENT_ARGUMENT);
  // patch message
  int send_message(const char* data, int len, int message_id);
  // clear
  void clear();

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