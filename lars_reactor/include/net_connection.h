#pragma once

/**
 * @brief 抽象连接类
 *
 */
class NetConnection {
 public:
  virtual int send_message(const char* data, int len, int message_id) = 0;
};