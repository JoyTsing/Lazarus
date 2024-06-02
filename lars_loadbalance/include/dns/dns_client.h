#pragma once

#include "eventloop/event_base.h"
#include "message/message.h"

namespace loadbalance {
/**
 * @brief 启动dns客户端
 *
 */
void start_dns_client();
namespace dns {
// main loop
void handle_dns_client();
// 有新的请求需要发送给dns server
void handle_dns_request(IO_EVENT_ARGUMENT);
// router
void handle_route_recv(MESSAGE_ROUTER_ARGS);
}  // namespace dns
}  // namespace loadbalance
