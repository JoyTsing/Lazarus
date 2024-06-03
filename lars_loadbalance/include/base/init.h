#pragma once

#include <memory>
#include <vector>

#include "balance/route_balance.h"
#include "lars.pb.h"
#include "utils/thread_queue.h"

namespace loadbalance {

struct LBConfig {
  // 经过若干次获取请求host节点后，试探选择一次overload过载节点
  int probe_num;
  // 初始化时一个节点的虚拟成功次数，防止启动少量失败就过载
  int init_success_cnt;
  int init_error_cnt;
  // idle节点失败率超过这个值就认为是overload节点
  float error_rate;
  // overload节点成功率超过这个值就认为是idle节点
  float success_rate;
  // 连续失败次数超过这个值就认为是overload节点
  int continue_error_num;
  // 连续成功次数超过这个值就认为是idle节点
  int continue_success_num;
};
namespace base {
// 所需要的队列
extern std::shared_ptr<ThreadQueue<lars::ReportStatusRequest>> reporter_queue;
extern std::shared_ptr<ThreadQueue<lars::GetRouterRequest>> dns_queue;
// 每个udp server对应的route balance
extern std::vector<std::shared_ptr<RouterBalance>> route_balances;
// 全局配置
extern LBConfig lb_config;
}  // namespace base
/**
 * @brief 资源初始化
 *
 */
void resource_init();
}  // namespace loadbalance
