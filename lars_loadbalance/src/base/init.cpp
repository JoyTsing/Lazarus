#include "base/init.h"

#include <memory>
#include <vector>

#include "utils/config_file.h"

namespace loadbalance {
namespace base {
std::shared_ptr<ThreadQueue<lars::ReportStatusRequest>> reporter_queue;
std::shared_ptr<ThreadQueue<lars::GetRouterRequest>> dns_queue;
std::vector<std::shared_ptr<RouterBalance>> route_balances;
LBConfig lb_config;
}  // namespace base
}  // namespace loadbalance

void loadbalance::resource_init() {
  config_file::setPath("./conf/lars_lb.conf");
  using namespace base;
  // 读取配置文件
  lb_config.probe_num =
      config_file::instance()->GetNumber("loadbalance", "probe_num", 10);
  lb_config.init_success_cnt = config_file::instance()->GetNumber(
      "loadbalance", "init_success_cnt", 180);
  lb_config.init_error_cnt =
      config_file::instance()->GetNumber("loadbalance", "init_error_cnt", 10);
  lb_config.error_rate =
      config_file::instance()->GetFloat("loadbalance", "error_rate", 0.1);
  lb_config.success_rate =
      config_file::instance()->GetFloat("loadbalance", "success_rate", 0.9);
  lb_config.continue_error_num = config_file::instance()->GetNumber(
      "loadbalance", "continue_error_num", 10);
  lb_config.continue_success_num = config_file::instance()->GetNumber(
      "loadbalance", "continue_success_num", 10);
  // 初始化队列
  reporter_queue = std::make_shared<ThreadQueue<lars::ReportStatusRequest>>();
  dns_queue = std::make_shared<ThreadQueue<lars::GetRouterRequest>>();
  for (int id = 1; id <= 3; id++) {
    route_balances.push_back(std::make_shared<RouterBalance>(id));
  }
}