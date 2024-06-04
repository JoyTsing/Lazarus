#pragma once

#include "eventloop/event_base.h"
#include "message/message.h"
#include "net/net_connection.h"

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
// dns链接成功时的回调
void handle_connection_init(NetConnection* conn, void* args);
// router
void handle_route_recv(MESSAGE_ROUTER_ARGS);
}  // namespace dns
}  // namespace loadbalance
