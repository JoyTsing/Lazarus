#include <netinet/in.h>
#include <unistd.h>

#include <cstdlib>
#include <mutex>
#include <random>
#include <thread>

#include "lars.pb.h"
#include "lazarus/lazarus.h"
#include "utils/minilog.h"

// developer can use the library by including the header file
using namespace lazarus;

std::once_flag print;
int cnt;

struct ID {
  ID() {
    modid = -1;
    cmdid = -1;
    print = false;
  }
  int modid;
  int cmdid;
  bool print;
};

std::mt19937 engine(std::random_device{}());
std::uniform_int_distribution<> dis(1, 3);

void test_qps(ID id) {
  auto client = std::make_shared<LazarusClient>();
  std::string ip;
  short port;

  int modid = id.modid;
  int cmdid = id.cmdid;
  // qps记录
  long qps = 0;
  // 记录最后时间
  long last_time = time(NULL);

  long total_qps = 0;
  long total_time_second = 0;
  int ret = client->subscribe(modid, cmdid);
  if (ret != lars::RET_SUCC) {
    minilog::log_info(
        "modid {} : cmdid {} still not exist host, after register.", modid,
        cmdid);
    if (dis(engine) < 2) {
      return;
    }
  }

  while (true) {
    std::call_once(print, [&]() { id.print = true; });
    ret = client->get_host(modid, cmdid, ip, port);
    if (ret == 0 || ret == 1 || ret == 3) {  // 成功,过载，不存在 均是合法返回
      ++qps;
      if (ret == 0) {
        client->report(modid, cmdid, ip, port, 0);  // 上报成功
      } else if (ret == 1) {
        client->report(modid, cmdid, ip, port, 1);  // 上报过载
      }
    } else {
      printf("[%d,%d] get error %d\n", modid, cmdid, ret);
    }

    // 当前时间
    long current_time = time(NULL);

    if (id.print && current_time - last_time >= 1) {
      total_time_second += 1;
      total_qps += qps;
      last_time = current_time;
      long avg_qps = (double)total_qps / total_time_second;
      long long total_avg_qps = avg_qps * cnt;
      printf("single qps = [%ld], average = [%ld], total QPS= [%lld]\n", qps,
             avg_qps, total_avg_qps);
      qps = 0;
    }
  }
}

int main(int argc, const char** argv) {
  if (argc != 2) {
    printf("Usage: ./qps [thread_num]\n");
    return -1;
  }
  cnt = atoi(argv[1]);
  std::vector<ID> ids(cnt);

  for (int i = 0; i < cnt; i++) {
    ids[i].modid = dis(engine);
    ids[i].cmdid = dis(engine);
    std::jthread(test_qps, ids[i]).detach();
  }
  std::this_thread::yield();
  pause();
  return 0;
}