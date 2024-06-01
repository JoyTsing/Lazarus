#pragma once

#include "eventloop/event_base.h"

/**
 * @brief 启动dns客户端
 *
 */
void start_dns_client();

void handle_dns_client();

void handle_dns_read(IO_EVENT_ARGUMENT);