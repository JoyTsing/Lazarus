#pragma once

#include "dns_router.h"
#include "message/message.h"

using client_subscribe_set = host_set;

// 周期性检测DB中的版本号是否有变化
void check_router_change(void* args);

// 获取路由信息
void get_router(MESSAGE_ROUTER_ARGS);

// publish测试
void publish_test();