#pragma once

#include <mysql/mysql.h>

#include <cstdint>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

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

  host_set get_hosts(int modid, int cmdid);

  /**
   * @brief load map-router from db
   *  TODO: 或许可以先加载到redis中，然后再从redis中加载
   *
   * @param is_bak 是否加载到bak_map中
   */
  void load_router_map(bool is_bak);

  /**
   * @brief load and check version is changed
   *
   * @return true : change
   * @return false : no change
   */
  bool check_version();

  /**
   * @brief load RouteChange table from db
   *
   * @param change_list
   */
  void load_changes(std::vector<std::uint64_t>& change_list);

  /**
   * @brief 将_router_map_bak 更新到 _router_map
   *
   */
  void update_router_map();

  void remove_changes(bool remove_all);

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
  std::mutex _mutex;
  MYSQL _db_connection;

  std::uint64_t _version;

  router_map _router_map;  // current router map
  router_map
      _router_map_bak;  // backup router map, used for updating router map
};