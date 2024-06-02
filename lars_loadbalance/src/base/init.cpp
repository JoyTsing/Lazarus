#include "base/init.h"

#include <memory>
#include <vector>

#include "utils/config_file.h"

namespace loadbalance {
namespace base {
std::shared_ptr<ThreadQueue<lars::ReportStatusRequest>> reporter_queue;
std::shared_ptr<ThreadQueue<lars::GetRouterRequest>> dns_queue;
std::vector<std::shared_ptr<RouterBalance>> route_balances;
}  // namespace base
}  // namespace loadbalance

void loadbalance::resource_init() {
  config_file::setPath("./conf/lars_lb.conf");
  base::reporter_queue =
      std::make_shared<ThreadQueue<lars::ReportStatusRequest>>();
  base::dns_queue = std::make_shared<ThreadQueue<lars::GetRouterRequest>>();
  for (int id = 1; id <= 3; id++) {
    base::route_balances.push_back(std::make_shared<RouterBalance>(id));
  }
}