#pragma once

#include <cstdint>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "eventloop/event_loop.h"

/**
 * @brief key: modid(32bit) /cmdid(32bit); value: set<fds>
 */
using subscribe_map =
    std::unordered_map<std::uint64_t, std::unordered_set<int>>;

/**
 * @brief key: fd, value: set<modid(32bit) /cmdid(32bit)>
 */
using publish_map = std::unordered_map<int, std::unordered_set<std::uint64_t>>;

class SubscribeList {
 public:
  static SubscribeList* instance() {
    static SubscribeList* instance = new SubscribeList();
    return instance;
  }

  /**
   * @brief 订阅消息
   *
   * @param key modid(high 32bit)|cmdid(low 32bit)
   * @param fd
   */
  void subscribe(std::uint64_t key, int fd);

  /**
   * @brief 取消订阅
   *
   * @param key modid(high 32bit)|cmdid(low 32bit)
   * @param fd
   */
  void unsubscribe(std::uint64_t key, int fd);

  /**
   * @brief 发布消息，通知对应的fd修改要寻找的模块
   *
   * @param change_mods 修改的modid/cmdid，用于过滤出对应的fd
   */
  void publish(const std::vector<int>& change_mods);

 private:
  SubscribeList();
  SubscribeList(const SubscribeList&) = delete;
  const SubscribeList& operator=(const SubscribeList&) = delete;

 private:
  /**
   * @brief 主动推送_publish_map里待发送的消息
   *
   * @param online_fds
   */
  void push_handler(const listen_fd_set& online_fds);

  /**
   * @brief 过滤出在线的fd
   *
   * @param online_fds
   */
  void filter_online_fds(const listen_fd_set& online_fds,
                         publish_map& need_publish);

 private:
  std::mutex _subscribe_mutex, _publish_mutex;
  subscribe_map _subscribe_map;
  publish_map _publish_map;
};