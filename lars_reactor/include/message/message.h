#pragma once

#include <cstdint>
#include <functional>
#include <iostream>
#include <unordered_map>

#include "net/net_connection.h"

using message_callback =
    std::function<void(const char* data, std::uint32_t len, int message_id,
                       NetConnection* client, void* user_data)>;

const constexpr short MESSAGE_HEAD_LEN = 8;
const constexpr int MESSAGE_LENGTH_LIMIT = 65527;

struct message_head {
  int message_id;
  int message_len;
};

// message router
class message_router {
 public:
  int register_router(int message_id, message_callback call_back, void* args) {
    if (_router.find(message_id) != _router.end()) {
      return -1;
    }
    std::cout << "register message_id: " << message_id << std::endl;
    _router[message_id] = call_back;
    _args[message_id] = args;
    return 0;
  }

  void call_router(int message_id, std::uint32_t msglen, const char* data,
                   NetConnection* conn) {
    if (_router.find(message_id) == _router.end()) {
      std::cout << "message_id: " << message_id << " not found" << std::endl;
      return;
    }
    auto callback = _router[message_id];
    callback(data, msglen, message_id, conn, _args[message_id]);
  }

 private:
  // router deliver
  std::unordered_map<int, message_callback> _router;
  std::unordered_map<int, void*> _args;
};