#pragma once

#include "message/message.h"

/**
 * @brief 启动UDP服务器
 *
 */

void start_udp_servers();

void handle_get_host(MESSAGE_ROUTER_ARGS);