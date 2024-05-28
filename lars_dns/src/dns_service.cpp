#include <mysql/mysql.h>

#include <cstring>

#include "eventloop/event_loop.h"
#include "net/tcp/tcp_server.h"
#include "utils/config_file.h"

TcpServer *server;

int main(int argc, const char **argv) {
  EventLoop loop;
  config_file::setPath("./conf/lars_dns.conf");

  std::string ip =
      config_file::instance()->GetString("reactor", "ip", "0.0.0.0");
  short port = config_file::instance()->GetNumber("reactor", "port", 7777);

  server = new TcpServer(&loop, ip.c_str(), port);
  // 测试mysql能否链接
  MYSQL *mysql = mysql_init(nullptr);
  loop.event_process();
  return 0;
}