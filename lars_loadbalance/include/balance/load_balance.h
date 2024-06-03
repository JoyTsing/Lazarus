#pragma once

#include <cstdint>
#include <list>
#include <memory>
#include <unordered_map>

#include "base/host_info.h"
#include "lars.pb.h"

class HostInfo;

using host_map = std::unordered_map<std::uint64_t, std::shared_ptr<HostInfo>>;
using host_map_it = host_map::iterator;
using host_list = std::list<std::shared_ptr<HostInfo>>;
using host_list_it = host_list::iterator;

/**
 * @brief LoadBalance class
 * Apply load rules to all host nodes under a module.
 * WARNING: This class is not thread-safe
 */
class LoadBalance {
 public:
  enum class Status : std::uint8_t {
    PULLING,  // 与dns server通信中
    NEW       // 正在创建LoadBalance模块
  };
  Status status;  // 当前状态

 public:
  LoadBalance(std::uint32_t modid, std::uint32_t cmdid);

  /**
   * @brief 拉取对应模块modid cmdid的所有host集合
   *
   */
  void pull();

  /**
   * @brief 根据DnsServer返回的host信息更新_host_map
   *
   */
  void update(const lars::GetRouterResponse& response);

  /**
   * @brief Get the one host object
   *
   * @param response
   * @return int //lars::ReturnCode
   */
  int get_one_host(lars::GetHostResponse& response);

  /**
   * @brief Report the host call info
   *
   * @param ip
   * @param port
   * @param retcode  // lars::ReturnCode
   */
  void report(int ip, int port, int retcode);

  /**
   * @brief Commit the report to server
   *
   */
  void commit_report();

  /**
   * @brief Check if the host list is empty
   *
   * @return true if empty
   * @return false if not empty
   */
  bool empty();

 private:
  /**
   * @brief Get the host from list object
   *
   * @param response
   * @param list
   */
  void get_host_from_list(lars::GetHostResponse& response, host_list& list);

 private:
  // belong to which module
  int _modid;
  int _cmdid;
  // host map,key is ip/port : value is HostInfo
  host_map _host_map;
  // idle
  host_list _idle_list;
  // overload
  host_list _overload_list;
  // modid/cmdid request times
  int _access_cnt;
};
