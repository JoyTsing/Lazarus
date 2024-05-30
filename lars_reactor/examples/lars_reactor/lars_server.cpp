#include <cstring>

#include "eventloop/event_loop.h"
#include "net/tcp/tcp_connection.h"
#include "net/tcp/tcp_server.h"
#include "utils/config_file.h"

TcpServer *server;

void print_lars_task(EventLoop *loop, void *args) {
  printf("======= Active Task Func! ========\n");
  listen_fd_set fds;
  // 当前在线的fd有哪些
  fds = loop->get_listen_fds();
  for (auto fd : fds) {
    TCPConnection *conn = TcpServer::get_connection(fd);
    if (conn == nullptr) {
      continue;
    }
    int msgid = 101;
    const char *msg = "Hello I am a Task!";
    conn->send_message(msg, strlen(msg), msgid);
  }
}
// 回显业务的回调函数
void echo(const char *data, std::uint32_t len, int msgid, NetConnection *conn,
          void *user_data) {
  // 直接回显
  conn->send_message(data, len, msgid);
  printf("conn param: %s\n", (char *)conn->param);
}

void print(const char *data, std::uint32_t len, int msgid, NetConnection *conn,
           void *user_data) {
  // 得到服务端回执的数据
  printf("  server callback handler2\n");
  printf("  recv from client: %s\n", data);
  printf("  msgid: [%d]\n", msgid);
  printf("  len: [%d]\n", len);
  printf("====================================\n");
}

// 新客户端创建后的回调函数
void on_client_build(NetConnection *conn, void *args) {
  int msgid = 200;
  const char *msg = "welcome to server";
  conn->send_message(msg, strlen(msg), msgid);
  // 每次用户创建链接成功之后执行一个任务
  server->get_threadpool()->send_task(print_lars_task);
  const char *conn_param = "come form conn_param";
  conn->param = (void *)conn_param;
}

// 客户端断开后的回调函数
void on_client_lost(NetConnection *conn, void *args) {
  printf("=====>client is lost\n");
}

int main(int argc, const char **argv) {
  EventLoop loop;
  config_file::setPath("./conf/server.conf");

  std::string ip =
      config_file::instance()->GetString("reactor", "ip", "0.0.0.0");
  short port = config_file::instance()->GetNumber("reactor", "port", 7777);

  server = new TcpServer(&loop, ip.c_str(), port);
  // 设置hook函数
  server->add_message_router(1, echo);
  server->add_message_router(2, print);
  server->add_message_router(101, print);
  server->add_message_router(200, print);
  // 设置连接hook函数
  server->set_conn_start_hook(on_client_build);
  server->set_conn_close_hook(on_client_lost);
  loop.event_process();
  return 0;
}