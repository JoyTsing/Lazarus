#pragma once

#include "eventloop/event_base.h"

namespace loadbalance {
/**
 * @brief 启动dns客户端
 *
 */
void start_dns_client();
namespace dns {
void handle_dns_client();
void handle_dns_read(IO_EVENT_ARGUMENT);
}  // namespace dns
}  // namespace loadbalance
