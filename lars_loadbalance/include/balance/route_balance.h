#pragma once
#include <memory>
#include <mutex>
#include <unordered_map>

#include "balance/load_balance.h"
#include "lars.pb.h"

class LoadBalance;

using router_map =
    std::unordered_map<std::uint64_t, std::shared_ptr<LoadBalance>>;
using router_map_it = router_map::iterator;

/**
 * @brief 针对多组mod/cmdid和load balance的对应关系
 * 现在的实现是每个udp server有一个router balance（TODO
 * 数量为3个，后续应该可以根据配置文件修改）
 */
class RouterBalance {
 public:
  RouterBalance(int id);

  /**
   * @brief Get the host object
   *
   * @param modid
   * @param cmdid
   * @param response
   * @return int lars::ReturnCode
   */
  int get_host(int modid, int cmdid, lars::GetHostResponse& response);

  /**
   * @brief Update the router_map from response
   *
   * @param modid
   * @param cmdid
   * @param response
   * @return int lars::ReturnCode
   */
  int update_host(int modid, int cmdid,
                  const lars::GetRouterResponse& response);

  /**
   * @brief report the host call info
   *
   * @param request
   */
  void report(const lars::ReportRequest& request);

 private:
  std::mutex _mtx;
  router_map _router_map;  // mod/cmdid和load balance的对应关系
  int _id;                 // router balance的id,和udp server的id对应
};