#include "reporter/report_client.h"

#include <thread>

#include "base/init.h"
#include "lars.pb.h"
#include "net/tcp/tcp_client.h"
#include "utils/config_file.h"
#include "utils/minilog.h"

void loadbalance::start_report_client() {
  // 启动一个线程，不断的从队列中取出数据，发送给reporter-server
  std::jthread([]() {
    minilog::log_info("report client start...");
    loadbalance::reporter::handle_report();
  }).detach();
}
void loadbalance::reporter::handle_reporter_read(IO_EVENT_ARGUMENT) {
  std::queue<lars::ReportStatusRequest> messages;
  TCPClient *client = (TCPClient *)args;
  loadbalance::base::reporter_queue->recv(messages);
  while (!messages.empty()) {
    lars::ReportStatusRequest request = messages.front();
    messages.pop();

    std::string request_string;
    request.SerializeToString(&request_string);

    // 发送dns-server
    client->send_message(request_string.c_str(), request_string.size(),
                         lars::ID_ReportStatusRequest);
  }
}

void loadbalance::reporter::handle_report() {
  EventLoop loop;
  // parser config
  std::string ip =
      config_file::instance()->GetString("reporter", "ip", "127.0.0.1");
  short port = config_file::instance()->GetNumber("reporter", "port", 9999);
  // client
  TCPClient client(&loop, ip.c_str(), port);
  // 让loop监控queue的数据
  loadbalance::base::reporter_queue->set_loop(&loop);
  loadbalance::base::reporter_queue->set_callback(handle_reporter_read,
                                                  &client);
  loop.event_process();
}