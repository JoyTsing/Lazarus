#pragma once

#include "message/message.h"

namespace loadbalance {
namespace server {
void handle_get_host(MESSAGE_ROUTER_ARGS);
void handle_get_report(MESSAGE_ROUTER_ARGS);
void handle_get_router(MESSAGE_ROUTER_ARGS);
}  // namespace server
/**
 * @brief 启动UDP服务器
 *
 */
void start_udp_servers();
}  // namespace loadbalance
