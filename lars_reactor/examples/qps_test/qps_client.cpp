#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <thread>
#include <vector>

#include "echoMessage.pb.h"
#include "eventloop/event_loop.h"
#include "net/net_connection.h"
#include "net/tcp/tcp_client.h"
#include "utils/minilog.h"

std::once_flag print;
int thread_num;
struct Qps {
  Qps() {
    last_time = time(NULL);
    succ_cnt = 0;
    print_QPS = false;
  }

  long last_time;  // 最后一次发包时间 ms为单位
  int succ_cnt;    // 成功收到服务器回显的次数
  bool print_QPS;
};

// 性能测试
void qps_test_handle(const char *data, std::uint32_t len, int msgid,
                     NetConnection *conn, void *user_data) {
  Qps *qps = (Qps *)user_data;
  qps_test::EchoMessage request, response;
  // 解析
  if (response.ParseFromArray(data, len) == false) {
    minilog::log_error("server call back data error");
    return;
  }
  // 判断内容
  if (response.content() == "hello world") {
    // qps success
    qps->succ_cnt++;
  }
  // 当前时间
  long current_time = time(NULL);
  if (qps->print_QPS && current_time - qps->last_time >= 1) {
    int server_qps = qps->succ_cnt * thread_num;
    printf("Client AVG QPS : [%d], ThreadNums : [%d], Server QPS: [%d]\n",
           qps->succ_cnt, thread_num, server_qps);
    qps->succ_cnt = 0;
    qps->last_time = current_time;
  }
  // 发送request给服务器
  request.set_id(response.id() + 1);
  request.set_content(response.content());

  std::string request_str;
  request.SerializeToString(&request_str);
  conn->send_message(request_str.c_str(), request_str.size(), msgid);
}

// 链接创建后的回调函数
void on_client_build(NetConnection *conn, void *args) {
  qps_test::EchoMessage request;
  request.set_id(1);
  request.set_content("hello world");
  std::string request_str;
  request.SerializeToString(&request_str);
  int msgid = 1;
  conn->send_message(request_str.c_str(), request_str.size(), msgid);
}

void thread_handle() {
  EventLoop loop;
  Qps qps;
  std::call_once(print, [&]() { qps.print_QPS = true; });
  TCPClient client(&loop, "127.0.0.1", 7777);
  client.add_message_router(1, qps_test_handle, (void *)&qps);
  client.set_construct_hook(on_client_build);
  loop.event_process();
}

int main(int argc, const char **argv) {
  if (argc == 1) {
    std::cout << "Usage : ./qps_client [threadNum]\n";
    return 0;
  }
  thread_num = atoi(argv[1]);
  std::vector<std::jthread> threads(thread_num);
  for (int i = 0; i < thread_num; i++) {
    threads[i] = std::jthread(thread_handle);
  }
  return 0;
}