#include "balance/route_balance.h"

#include <mutex>

#include "balance/load_balance.h"

RouterBalance::RouterBalance(int id) : _id(id) {}

int RouterBalance::get_host(int modid, int cmdid,
                            lars::GetHostResponse& response) {
  int ret = lars::RET_SUCC;
  // 1. 根据modid和cmdid计算hash值
  std::uint64_t key = ((std::uint64_t)modid << 32) + cmdid;
  // 2. 根据hash值找到对应的load balance
  std::lock_guard<std::mutex> lock(_mtx);
  if (_router_map.find(key) != _router_map.end()) {
    auto lb = _router_map[key];
    // 3.1 根据load balance获取host
    // TODO
  } else {
    // 3.1 没有找到对应的load balance
    auto lb = std::make_shared<LoadBalance>(modid, cmdid);
    _router_map[key] = lb;
    // 3.2 前往dns_server 拉取对应模块的所有host集合
    lb->pull();
    // 3.3设置返回值
    response.set_retcode(lars::RET_NOEXIST);
    ret = lars::RET_NOEXIST;
  }
  return ret;
}

int RouterBalance::update_host(int modid, int cmdid,
                               const lars::GetRouterResponse& response) {
  int ret = lars::RET_SUCC;
  // 1. 根据modid和cmdid计算hash值
  std::uint64_t key = ((std::uint64_t)modid << 32) + cmdid;
  // 2. 根据hash值找到对应的load balance
  std::lock_guard<std::mutex> lock(_mtx);
  if (_router_map.find(key) != _router_map.end()) {
    auto lb = _router_map[key];
    if (response.hosts_size() == 0) {
      // 3.1 没有host信息,删除对应的load balance
      lb.reset();
      _router_map.erase(key);
    } else {
      // 3.2 更新load balance的host信息
      lb->update(response);
    }
  }
  // 3. 更新load balance的host信息
  return ret;
}