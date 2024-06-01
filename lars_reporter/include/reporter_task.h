#pragma once
#include "message/message.h"

void get_reporter_status(MESSAGE_ROUTER_ARGS);

/**
 * @brief start a thread pool to handle the data to disk
 *
 *
 */
void create_store_threadpool();

/**
 * @brief 用于处理磁盘IO的主事件循环
 *
 * @param index
 */
void store_handler(int index);