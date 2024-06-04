#include "balance/route_balance.h"

#include <cstdint>
#include <mutex>

#include "balance/load_balance.h"
#include "lars.pb.h"

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
    if (lb->empty()) {
      // 没有host信息,说明可能正在拉取host信息
      response.set_retcode(lars::RET_NOEXIST);
    } else {
      // 获取host
      ret = lb->get_one_host(response);
      response.set_retcode(ret);
    }
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

void RouterBalance::report(const lars::ReportRequest& request) {
  // 1. 根据modid和cmdid找到对应的load balance
  int modid = request.modid(), cmdid = request.cmdid();
  int retcode = request.retcode();
  std::uint32_t ip = request.host().ip(), port = request.host().port();
  std::uint64_t key = ((std::uint64_t)modid << 32) + cmdid;
  // 2. load balance 处理report请求
  std::lock_guard<std::mutex> lock(_mtx);
  if (_router_map.find(key) != _router_map.end()) {
    auto lb = _router_map[key];
    // 2.1 通过对应主机的上报情况，更新lb内部的状态
    lb->report(ip, port, retcode);
    // 2.2 根据最终结果上报report server
    lb->commit_report();
  }
}