#include <chrono>
#include <ctime>

#include "eventloop/event_loop.h"
#include "lars.pb.h"
#include "net/net_connection.h"
#include "net/tcp/tcp_client.h"
#include "utils/minilog.h"

void report_host_status(NetConnection* conn, void* args) {
  lars::ReportStatusRequest request;
  request.set_modid(1);
  request.set_cmdid(1);
  request.set_caller(1145141919);
  request.set_timestamp(time(NULL));
  for (int i = 0; i < 3; i++) {
    lars::HostCallResult result;
    result.set_ip(i + 1);
    result.set_port(i + 1000);
    result.set_succ(i * 100);
    result.set_fail(i * 10);
    result.set_overload(true);

    request.add_results()->CopyFrom(result);
  }
  std::string request_str;
  request.SerializeToString(&request_str);
  conn->send_message(request_str.c_str(), request_str.size(),
                     lars::ID_ReportStatusRequest);
}

int main(int argc, const char** argv) {
  EventLoop loop;
  TCPClient* client = new TCPClient(&loop, "127.0.0.1", 7779);
  client->set_construct_hook(report_host_status);

  loop.event_process();
  return 0;
}