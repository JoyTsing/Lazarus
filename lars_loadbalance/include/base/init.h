#pragma once

#include <memory>

#include "lars.pb.h"
#include "utils/thread_queue.h"

// 所需要的队列
extern std::shared_ptr<ThreadQueue<lars::ReportStatusRequest>> reporter_queue;
extern std::shared_ptr<ThreadQueue<lars::GetRouterRequest>> dns_queue;

void resource_init();