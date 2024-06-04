#include <netinet/in.h>
#include <unistd.h>

#include <cstdlib>
#include <memory>

#include "lars.pb.h"
#include "lazarus/lazarus.h"
#include "utils/minilog.h"

// developer can use the library by including the header file
using namespace lazarus;

int main(int argc, const char** argv) {
  if (argc != 4) {
    printf("Usage: ./example [modid] [cmdid] [overload]\n");
    return -1;
  }

  auto client = std::make_shared<LazarusClient>();
  std::string ip;
  short port;

  int modid = atoi(argv[1]), cmdid = atoi(argv[2]), overload = atoi(argv[3]);

  // int ret = client->get_host(modid, cmdid, ip, port);
  // if (ret == lars::RET_SUCC) {
  //   minilog::log_info("host is {}, port is {}", ip, port);
  //   // do something
  //   client->report(modid, cmdid, ip, port, overload);
  //   // report
  // } else if (ret == lars::RET_NOEXIST) {
  //   minilog::log_info("no exist");
  // }

  lazarus::router_set routers;
  int ret = client->get_routers(modid, cmdid, routers);
  if (ret == lars::RET_SUCC) {
    for (auto& router : routers) {
      minilog::log_info("host is {}, port is {}", router.first, router.second);
    }
  } else {
    minilog::log_error("get routers error");
  }
  return 0;
}