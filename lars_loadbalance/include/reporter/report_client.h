#pragma once

#include "eventloop/event_base.h"

namespace loadbalance {
/**
 * @brief 启动reporter客户端
 *
 */
void start_report_client();
namespace reporter {
void handle_report();
void handle_reporter_read(IO_EVENT_ARGUMENT);
}  // namespace reporter
}  // namespace loadbalance
