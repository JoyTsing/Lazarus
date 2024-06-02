#include "base/init.h"

#include "utils/config_file.h"

std::shared_ptr<ThreadQueue<lars::ReportStatusRequest>>
    loadbalance::base::reporter_queue;
std::shared_ptr<ThreadQueue<lars::GetRouterRequest>>
    loadbalance::base::dns_queue;

void loadbalance::resource_init() {
  config_file::setPath("./conf/lars_lb.conf");
  loadbalance::base::reporter_queue =
      std::make_shared<ThreadQueue<lars::ReportStatusRequest>>();
  loadbalance::base::dns_queue =
      std::make_shared<ThreadQueue<lars::GetRouterRequest>>();
}