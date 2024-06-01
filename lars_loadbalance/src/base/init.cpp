#include "base/init.h"

#include "utils/config_file.h"

std::shared_ptr<ThreadQueue<lars::ReportStatusRequest>> reporter_queue;
std::shared_ptr<ThreadQueue<lars::GetRouterRequest>> dns_queue;

void resource_init() {
  config_file::setPath("./conf/lars_lb.conf");
  reporter_queue = std::make_shared<ThreadQueue<lars::ReportStatusRequest>>();
  dns_queue = std::make_shared<ThreadQueue<lars::GetRouterRequest>>();
}