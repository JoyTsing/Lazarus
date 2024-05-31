#include "dns_task.h"

#include <thread>

#include "dns_router.h"
#include "dns_subscribe.h"
#include "lars.pb.h"
#include "utils/minilog.h"

// 获取路由信息
void get_router(const char* data, std::uint32_t len, int message_id,
                NetConnection* conn, void* user_data) {
  // 1. 解析proto
  lars::GetRouterRequest request;
  request.ParseFromArray(data, len);
  // 2. get modID & cmdID
  int modid, cmdid;
  modid = request.modid();
  cmdid = request.cmdid();
  // 2.5 订阅
  std::uint64_t mod = ((std::uint64_t)modid << 32) + cmdid;
  client_subscribe_set* set = (client_subscribe_set*)conn->param;
  if (set == nullptr) {
    minilog::log_error("subscribe set is nullptr");
  } else if (set->find(mod) == set->end()) {
    SubscribeList::instance()->subscribe(mod, conn->get_fd());
    set->insert(mod);
    // minilog::log_info(" @fd [{}] subscribe: modid = {}, cmdid= {}",
    //                 conn->get_fd(), modid, cmdid);
  }
  // 3. parse to host
  host_set hosts = Router::instance()->get_hosts(modid, cmdid);
  // 4. response
  lars::GetRouterResponse response;
  response.set_modid(modid);
  response.set_cmdid(cmdid);
  for (auto host : hosts) {
    lars::HostInfo host_info;
    host_info.set_ip((std::uint32_t)(host >> 32));
    host_info.set_port((int)host);
    // add to response
    response.add_hosts()->CopyFrom(host_info);
    // minilog::log_info("   host ip: {}, port: {}",
    // (std::uint32_t)host_info.ip(),
    //                   host_info.port());
  }
  // 5. send
  std::string send_data;
  response.SerializeToString(&send_data);
  conn->send_message(send_data.c_str(), send_data.size(),
                     lars::ID_GetRouterResponse);
}

void handler_change() {
  // 1 将最新的RouteData数据加载到_router_map_bak
  Router::instance()->load_router_map(true);
  // 2 将_router_map_bak 更新到 _router_map
  Router::instance()->update_router_map();
  // 3 获取变化的modID/cmdID列表
  std::vector<std::uint64_t> change_list;
  Router::instance()->load_changes(change_list);
  // 4 通知所有订阅过修改mod的订阅者
  SubscribeList::instance()->publish(change_list);
  // 5 清空无效的RouteChange表
  // Router::instance()->remove_changes(false);
}

// 周期性检测DB中的版本号是否有变化
void check_router_change(void* args) {
  std::chrono::seconds duration(10);
  // 版本初始化
  Router::instance()->check_version();
  // 每次启动服务器将老的版本号去掉
  Router::instance()->remove_changes(true);
  std::chrono::system_clock::time_point last_time =
      std::chrono::system_clock::now();
  while (true) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    bool is_change = Router::instance()->check_version();
    std::chrono::system_clock::time_point now =
        std::chrono::system_clock::now();
    // 检测版本号是否有变化
    if (is_change) {
      // 有变化
      handler_change();
      last_time = now;
    } else {
      // 无变化
      if (now - last_time >= duration) {
        // 加载最新的router_map
        Router::instance()->load_router_map(true);
        Router::instance()->update_router_map();
        last_time = now;
        // TODO 可以在这里增添心跳机制
      }
    }
  }
}

void publish_test() {
  while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    // modid =1,cmdid=1
    int modid = 1, cmdid = 1;
    std::uint64_t mod = ((std::uint64_t)modid << 32) + cmdid;
    // modid =1,cmdid=2
    int modid2 = 1, cmdid2 = 2;
    std::uint64_t mod2 = ((std::uint64_t)modid2 << 32) + cmdid2;
    std::vector<std::uint64_t> change_mods = {mod, mod2};
    SubscribeList::instance()->publish(change_mods);
  }
}