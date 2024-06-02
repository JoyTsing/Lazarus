#pragma once

#include <memory>

#include "lars.pb.h"
#include "utils/thread_queue.h"

namespace loadbalance {
namespace base {
// 所需要的队列
extern std::shared_ptr<ThreadQueue<lars::ReportStatusRequest>> reporter_queue;
extern std::shared_ptr<ThreadQueue<lars::GetRouterRequest>> dns_queue;
}  // namespace base
/**
 * @brief 资源初始化
 *
 */
void resource_init();
}  // namespace loadbalance
