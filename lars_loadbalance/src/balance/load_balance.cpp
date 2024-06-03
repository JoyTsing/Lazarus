#include "balance/load_balance.h"

#include <cassert>
#include <cstdint>
#include <unordered_set>

#include "base/host_info.h"
#include "base/init.h"
#include "lars.pb.h"
#include "utils/minilog.h"

LoadBalance::LoadBalance(std::uint32_t modid, std::uint32_t cmdid)
    : _modid(modid), _cmdid(cmdid), _access_cnt(0) {}

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

int LoadBalance::get_one_host(lars::GetHostResponse& response) {
  // 1. _idle_list is empty ?
  if (_idle_list.empty() == true) {
    // 1.1 if not empty and not exceed probe_num
    if (_access_cnt >= loadbalance::base::lb_config.probe_num) {
      // 1.2 try get host from _overload_list
      get_host_from_list(response, _overload_list);
      _access_cnt = 0;
    } else {
      _access_cnt++;
      return lars::RET_OVERLOAD;
    }
  } else {
    // 2 _idle_list not empty
    if (!_overload_list.empty() &&
        _access_cnt >= loadbalance::base::lb_config.probe_num) {
      // 2.1 try get host from _overload_list
      get_host_from_list(response, _overload_list);
      _access_cnt = 0;
    } else {
      // 2.2 get host from _idle_list
      get_host_from_list(response, _idle_list);
      _access_cnt++;
    }
  }

  return lars::RET_SUCC;
}

void LoadBalance::get_host_from_list(lars::GetHostResponse& response,
                                     host_list& list) {
  auto host = list.front();
  auto host_info = response.mutable_hosts();
  host_info->set_ip(host->ip);
  host_info->set_port(host->port);
  // move to back
  list.pop_front();
  list.push_back(host);
}

bool LoadBalance::empty() { return _host_map.empty(); }

void LoadBalance::report(int ip, int port, int retcode) {
  minilog::log_info("load balance report");
}

void LoadBalance::commit_report() {
  minilog::log_info("load balance commit report");
}