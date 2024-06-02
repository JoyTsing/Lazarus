#include "balance/load_balance.h"

#include <cassert>
#include <cstdint>
#include <unordered_set>

#include "base/init.h"
#include "lars.pb.h"

LoadBalance::LoadBalance(std::uint32_t modid, std::uint32_t cmdid)
    : _modid(modid), _cmdid(cmdid) {}

void LoadBalance::pull() {
  // 1. dns request
  lars::GetRouterRequest request;
  request.set_modid(_modid);
  request.set_cmdid(_cmdid);
  // 2. dns response
  loadbalance::base::dns_queue->send(request);
  status = Status::PULLING;
}

void LoadBalance::update(const lars::GetRouterResponse& response) {
  // response 的host信息不为空
  assert(response.hosts_size() > 0);
  std::unordered_set<std::uint64_t> remote_set;
  std::unordered_set<std::uint64_t> delete_set;
  // 1. 插入新增host
  for (int i = 0; i < response.hosts_size(); i++) {
    const lars::HostInfo& host = response.hosts(i);
    // 1.1 host key
    std::uint64_t key = ((std::uint64_t)host.ip() << 32) + host.port();
    // 1.2 host info
    remote_set.insert(key);
    if (_host_map.find(key) == _host_map.end()) {
      _host_map[key] = std::make_shared<HostInfo>(host.ip(), host.port());
      // add to idle list
      _idle_list.push_back(_host_map[key]);
    }
  }
  // 2. 远程与本地交集，需要删除减少的host
  for (host_map_it it = _host_map.begin(); it != _host_map.end(); it++) {
    if (remote_set.find(it->first) == remote_set.end()) {
      delete_set.insert(it->first);
    }
  }
  // 3. 删除host
  for (std::uint64_t key : delete_set) {
    auto host = _host_map[key];
    if (host->overload == true) {
      // 3.1 从overload list中删除
      _overload_list.remove(host);
    } else {
      // 3.2 从idle list中删除
      _idle_list.remove(host);
    }
    // 3.3 从host map中删除
    _host_map.erase(key);
    host.reset();
  }
}