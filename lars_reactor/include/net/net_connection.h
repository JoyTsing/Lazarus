#pragma once

#include <functional>

/**
 * @brief 抽象连接类
 *
 */
class NetConnection {
 public:
  virtual ~NetConnection() = default;
  virtual int send_message(const char* data, int len, int message_id) = 0;
  virtual int get_fd() = 0;
  void* param;  // 用来存放一些额外的动态参数
};

// construct and destruct hook function
using connection_callback =
    std::function<void(NetConnection* conn, void* args)>;