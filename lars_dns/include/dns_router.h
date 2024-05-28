#pragma once

#include <mysql/mysql.h>

#include <cstdint>
#include <unordered_map>
#include <unordered_set>

// 定义用来保存host的IP/host的port的的集合数据类型

using host_set = std::unordered_set<std::uint64_t>;
using host_set_iter = host_set::iterator;

// 用于保存modID/cmdID到host:port的映射关系
// key: 高32位存modID，低32位存cmdID
// value : 高32位ip，低32位port
using router_map = std::unordered_map<std::uint64_t, host_set>;
using router_map_iter = router_map::iterator;

/**
 * @brief modid(模块id)/cmdid(服务id) -> host:ip,host:port的对应关系
 *
 */
class Router {
 public:
  static Router* instance() {
    static Router* instance = new Router();
    return instance;
  }

  /**
   * @brief load map-router from db
   *
   */
  void load_router_map();

 private:
  Router();
  Router(const Router&) = delete;
  const Router& operator=(const Router&) = delete;

  /**
   * @brief 链接mysql数据库
   *
   */
  void connect_db();

 private:
  MYSQL _db_connection;
  router_map _router_map;  // current router map
  router_map
      _router_map_bak;  // backup router map, used for updating router map
};