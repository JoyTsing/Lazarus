#include "balance/load_balance.h"

#include <arpa/inet.h>
#include <netinet/in.h>

#include <cassert>
#include <cstdint>
#include <ctime>
#include <unordered_set>

#include "base/host_info.h"
#include "base/init.h"
#include "lars.pb.h"
#include "utils/minilog.h"

LoadBalance::LoadBalance(std::uint32_t modid, std::uint32_t cmdid)
    : _modid(modid),
      _cmdid(cmdid),
      _access_cnt(0),
      status(Status::NEW),
      _update_time(0) {
  ;
}

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
      _host_map[key] = std::make_shared<HostInfo>(
          host.ip(), host.port(),
          loadbalance::base::lb_config.init_success_cnt);
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
  // 更新最后的update时间并且重置状态
  _update_time = time(nullptr);
  status = Status::NEW;
}

void LoadBalance::get_hosts(lars::GetRouterResponse& response) {
  for (auto [_, hostinfo] : _host_map) {
    lars::HostInfo host;
    host.set_ip(hostinfo->ip);
    host.set_port(hostinfo->port);
    response.add_hosts()->CopyFrom(host);
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

std::uint64_t LoadBalance::get_update_time() const { return _update_time; }

// TODO 这个函数太长了，需要拆分
void LoadBalance::report(std::uint32_t ip, int port, int retcode) {
  uint64_t key = ((uint64_t)ip << 32) + port;
  if (_host_map.find(key) == _host_map.end()) {
    return;
  }
  std::uint64_t current_time = time(nullptr);
  auto hostinfo = _host_map[key];
  // 1. update count
  if (retcode == lars::RET_SUCC) {
    hostinfo->virtual_succ++;
    hostinfo->real_succ++;
    hostinfo->continue_succ++;
    hostinfo->continue_err = 0;
  } else {
    hostinfo->virtual_err++;
    hostinfo->real_err++;
    hostinfo->continue_err++;
    hostinfo->continue_succ = 0;
  }
  // 2. check overload
  if (hostinfo->overload == false && retcode != lars::RET_SUCC) {
    // idle -> overload
    bool overload = false;
    double err_rate = (double)hostinfo->virtual_err /
                      (hostinfo->virtual_succ + hostinfo->virtual_err);
    // 2.1 check error rate
    if (err_rate > loadbalance::base::lb_config.error_rate) {
      // overload
      overload = true;
    }
    // 2.2 check continue error
    if (overload == false &&
        hostinfo->continue_err >=
            (std::uint32_t)loadbalance::base::lb_config.continue_error_num) {
      // overload
      overload = true;
    }
    // 2.3 update overload status
    if (overload) {
      in_addr addr;
      addr.s_addr = htonl(hostinfo->ip);
      minilog::log_info(
          "[modid {}:cmdid {}] host {}:{} overload, vir_success {}, vir_error "
          "{}",
          _modid, _cmdid, inet_ntoa(addr), hostinfo->port,
          hostinfo->virtual_err, hostinfo->virtual_err);
      hostinfo->set_overload();
      _idle_list.remove(hostinfo);
      _overload_list.push_back(hostinfo);
      return;
    }
  } else if (hostinfo->overload == true && retcode == lars::RET_SUCC) {
    // overload -> idle
    bool idle = false;
    double succ_rate = (double)hostinfo->virtual_succ /
                       (hostinfo->virtual_succ + hostinfo->virtual_err);
    if (succ_rate > loadbalance::base::lb_config.success_rate) {
      idle = true;
    }
    if (idle == false &&
        hostinfo->continue_succ >=
            (std::uint32_t)loadbalance::base::lb_config.continue_success_num) {
      idle = true;
    }

    if (idle) {
      in_addr addr;
      addr.s_addr = htonl(hostinfo->ip);
      minilog::log_info(
          "[modid {}:cmdid {}] host {}:{} idle, vir_success {}, vir_error "
          "{}",
          _modid, _cmdid, inet_ntoa(addr), hostinfo->port,
          hostinfo->virtual_err, hostinfo->virtual_err);
      hostinfo->set_idle();
      _overload_list.remove(hostinfo);
      _idle_list.push_back(hostinfo);
      return;
    }
  }
  // 3. 周期检查
  if (hostinfo->overload == false) {
    // idle
    if (current_time - hostinfo->idle_timestamp >=
        (std::uint64_t)loadbalance::base::lb_config.idle_timeout) {
      // 重置
      minilog::log_info("[modid {}:cmdid {}] idle reset", _modid, _cmdid);
      hostinfo->set_idle();
    }
  } else {
    // overload
    if (current_time - hostinfo->overload_timestamp >=
        (std::uint64_t)loadbalance::base::lb_config.overload_timeout) {
      // 重置
      minilog::log_info("[modid {}:cmdid {}] overload reset", _modid, _cmdid);
      hostinfo->set_idle();
      _overload_list.remove(hostinfo);
      _idle_list.push_back(hostinfo);
    }
  }
}

void LoadBalance::commit_report() {
  if (empty()) {
    return;
  }
  // 1.  packaging information
  lars::ReportStatusRequest request;
  request.set_modid(_modid);
  request.set_cmdid(_cmdid);
  request.set_timestamp(time(nullptr));
  request.set_caller(loadbalance::base::lb_config.local_ip);
  // 2. report _idle_list
  for (auto hostinfo : _idle_list) {
    lars::HostCallResult call_res;
    call_res.set_ip(hostinfo->ip);
    call_res.set_port(hostinfo->port);
    call_res.set_succ(hostinfo->real_succ);
    call_res.set_fail(hostinfo->real_err);
    call_res.set_overload(false);
    // add to req
    request.add_results()->CopyFrom(call_res);
  }
  // 3. report _overload_list
  for (auto hostinfo : _overload_list) {
    lars::HostCallResult call_res;
    call_res.set_ip(hostinfo->ip);
    call_res.set_port(hostinfo->port);
    call_res.set_succ(hostinfo->real_succ);
    call_res.set_fail(hostinfo->real_err);
    call_res.set_overload(true);
    // add to req
    request.add_results()->CopyFrom(call_res);
  }
  // 4.send
  loadbalance::base::reporter_queue->send(request);
}