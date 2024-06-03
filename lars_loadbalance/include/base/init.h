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
