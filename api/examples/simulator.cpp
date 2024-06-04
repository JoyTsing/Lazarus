#include <netinet/in.h>
#include <unistd.h>

#include <cstdint>
#include <cstdlib>
#include <memory>
#include <thread>
#include <unordered_map>
#include <utility>

#include "lars.pb.h"
#include "lazarus/lazarus.h"
#include "utils/minilog.h"

// developer can use the library by including the header file
using namespace lazarus;

void usage() {
  printf(
      "Usage: ./example [modid] [cmdid] [errRate(0-10)] [query "
      "cnt(0-999999)]\n");
}

int main(int argc, const char** argv) {
  if (argc < 3) {
    usage();
    return -1;
  }
  int modid = atoi(argv[1]), cmdid = atoi(argv[2]);
  int err_rate = 2;      // default
  int query_cnt = 1000;  // default
  if (argc > 3) {
    err_rate = atoi(argv[3]);
  }
  if (argc > 4) {
    query_cnt = atoi(argv[4]);
  }
  int ret;
  auto client = std::make_shared<LazarusClient>();
  std::string ip;
  short port;
  minilog::log_info("err_rate = {}, query_cnt = {}", err_rate, query_cnt);
  // key ->ip ;value -> <suc_cnt,err_cnt>
  std::unordered_map<std::string, std::pair<std::uint32_t, std::uint32_t>> mp;
  for (int i = 0; i < query_cnt; i++) {
    ret = client->get_host(modid, cmdid, ip, port);
    if (ret == 0) {
      if (mp.find(ip) == mp.end()) {
        mp[ip] = std::make_pair(0, 0);
      }
      minilog::log_info("host is {}, port is {}", ip, port);
      // do something
      if (rand() % 10 < err_rate) {
        // report error
        mp[ip].second++;
        client->report(modid, cmdid, ip, port, lars::RET_OVERLOAD);
        // minilog::log_info("error");
      } else {
        // report success
        mp[ip].first++;
        client->report(modid, cmdid, ip, port, lars::RET_SUCC);
        // minilog::log_info("success");
      }
    } else if (ret == lars::RET_NOEXIST) {
      minilog::log_info("modid {},cmdid {} no exist", modid, cmdid);
    } else if (ret == lars::RET_OVERLOAD) {
      // minilog::log_info("all host overload");
    } else if (ret == lars::RET_SYSTEM_ERR) {
      minilog::log_info("SYSTEM error");
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  // print result
  for (auto& kv : mp) {
    minilog::log_info("ip:{} success:{} error:{}", kv.first, kv.second.first,
                      kv.second.second);
  }
  return 0;
}