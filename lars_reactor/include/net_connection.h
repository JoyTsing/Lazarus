#pragma once

#include <functional>

/**
 * @brief 抽象连接类
 *
 */
class NetConnection {
 public:
  virtual int send_message(const char* data, int len, int message_id) = 0;
};

// construct and destruct hook function
using connection_callback =
    std::function<void(NetConnection* conn, void* args)>;