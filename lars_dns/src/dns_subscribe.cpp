#include "dns_subscribe.h"

#include <cstdint>
#include <mutex>

#include "dns_router.h"
#include "eventloop/event_loop.h"
#include "lars.pb.h"
#include "net/net_connection.h"
#include "net/tcp/tcp_server.h"
#include "utils/minilog.h"

// 全局的server
extern TcpServer* server;

SubscribeList::SubscribeList() {}

void SubscribeList::subscribe(std::uint64_t key, int fd) {
  std::lock_guard<std::mutex> lock(_subscribe_mutex);
  _subscribe_map[key].insert(fd);
}

void SubscribeList::unsubscribe(std::uint64_t key, int fd) {
  std::lock_guard<std::mutex> lock(_subscribe_mutex);
  if (auto it = _subscribe_map.find(key); it != _subscribe_map.end()) {
    _subscribe_map[key].erase(fd);
    if (_subscribe_map[key].empty()) {
      _subscribe_map.erase(key);
    }
  }
}

void SubscribeList::filter_online_fds(const listen_fd_set& online_fds,
                                      publish_map& need_publish) {
  std::lock_guard<std::mutex> lock(_publish_mutex);

  for (auto it = _publish_map.begin(); it != _publish_map.end();) {
    auto [fd, _] = *it;
    if (online_fds.find(fd) != online_fds.end()) {
      // need_publish[fd] = _publish_map[fd]; 最好不要用浅拷贝
      for (auto mod : _publish_map[fd]) {
        need_publish[fd].insert(mod);
      }
      _publish_map.erase(it++);
    } else {
      // 调用erase时候出问题了
      it++;
    }
  }
}

void SubscribeList::push_handler(const listen_fd_set& online_fds) {
  // 将 _publish_map中寻找需要修改的部分
  publish_map need_publish;
  filter_online_fds(online_fds, need_publish);
  // 将过滤出来的部分发送给对应的fd
  for (const auto& [fd, modify_set] : need_publish) {
    // fd对应的修改过的modid/cmdid集合
    for (auto mod : modify_set) {
      int modid = (int)(mod >> 32);
      int cmdid = (int)(mod);
      // proto-reply
      lars::GetRouterResponse response;
      response.set_modid(modid);
      response.set_cmdid(cmdid);
      // 通过router获取对应的ip:port并封装到response中
      host_set hosts = Router::instance()->get_hosts(modid, cmdid);
      for (const auto& host : hosts) {
        lars::HostInfo host_info;
        host_info.set_ip((std::uint32_t)(host >> 32));
        host_info.set_port((std::uint32_t)(host));
        // 将host_info加入到response中
        response.add_hosts()->CopyFrom(host_info);
      }
      // 将response序列化
      std::string response_str;
      response.SerializeToString(&response_str);
      // 将response_str发送给fd
      NetConnection* conn = TcpServer::get_connection(fd);
      if (conn != nullptr) {
        conn->send_message(response_str.c_str(), response_str.size(),
                           lars::ID_GetRouterResponse);
      }
    }
    // 发送
  }
}

void SubscribeList::publish(const std::vector<std::uint64_t>& change_mods) {
  {
    // 临界资源加锁
    std::lock_guard<std::mutex> lock(_subscribe_mutex);
    std::lock_guard<std::mutex> lock2(_publish_mutex);
    for (auto mod : change_mods) {
      std::uint64_t key = mod;
      if (auto it = _subscribe_map.find(key); it != _subscribe_map.end()) {
        // 向需要publish的fd中加入要修改的modid/cmdid
        for (auto fd : _subscribe_map[key]) {
          _publish_map[fd].insert(key);
        }
      }
    }
  }
  // 通知对应的fd修改要寻找的模块
  server->get_threadpool()->send_task(
      [](EventLoop* loop, void* args) {
        SubscribeList* list = (SubscribeList*)args;
        list->push_handler(loop->get_listen_fds());
      },
      this);
}
