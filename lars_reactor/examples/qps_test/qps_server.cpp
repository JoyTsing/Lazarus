#include <cstring>

#include "echoMessage.pb.h"
#include "net/tcp/tcp_server.h"
#include "utils/config_file.h"

void qps_test_handle(const char *data, std::uint32_t len, int msgid,
                     NetConnection *conn, void *user_data) {
  // proto解包
  qps_test::EchoMessage request, response;
  request.ParseFromArray(data, len);
  // minilog::log_info("content = {}\nid={}", request.content(), request.id());
  // 赋值
  response.set_id(request.id());
  response.set_content(request.content());
  // 序列化
  std::string response_str;
  response.SerializeToString(&response_str);
  // 发送
  conn->send_message(response_str.c_str(), response_str.size(), msgid);
}

int main(int argc, const char **argv) {
  EventLoop loop;
  config_file::setPath("./conf/server.conf");

  std::string ip =
      config_file::instance()->GetString("reactor", "ip", "127.0.0.1");
  short port = config_file::instance()->GetNumber("reactor", "port", 7777);

  TcpServer server(&loop, ip.c_str(), port);
  // 设置hook函数
  server.add_message_router(1, qps_test_handle);
  loop.event_process();
  return 0;
}