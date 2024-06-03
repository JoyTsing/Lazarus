#include <unistd.h>

#include <cstdlib>
#include <memory>

#include "lars.pb.h"
#include "lazarus/lazarus.h"
#include "utils/minilog.h"

// developer can use the library by including the header file
using namespace lazarus;

int main(int argc, const char** argv) {
  if (argc != 3) {
    printf("Usage: ./example [modid] [cmdid]\n");
    return -1;
  }

  auto client = std::make_shared<LazarusClient>();
  std::string ip;
  short port;

  int modid = atoi(argv[1]), cmdid = atoi(argv[2]);

  int ret = client->get_hosts(modid, cmdid, ip, port);
  if (ret == lars::RET_SUCC) {
    minilog::log_info("host is {}, port is {}", ip, port);
    // do something
    // report
  }
  return 0;
}