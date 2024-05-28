#pragma once
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <cstdint>
#include <mutex>
#include <unordered_map>

#include "eventloop/event_loop.h"
#include "message/message.h"
#include "net/tcp/tcp_connection.h"
#include "utils/threadpool.h"

class TcpServer {
 public:
  friend class TCPConnection;

  TcpServer(EventLoop* loop, const char* ip, std::uint16_t port);
  ~TcpServer();

  void handle_accept();

  void add_message_router(int msg_id, message_callback handler,
                          void* args = nullptr);

  // connections
  static void add_connection(int conn_fd, TCPConnection* conn);
  static void remove_connection(int conn_fd);
  static int get_connection_num();
  // get connection by fd
  static TCPConnection* get_connection(int conn_fd);
  // threadpool
  ThreadPool* get_threadpool();

  /**
   * @brief 用来处理注册过的消息的路由函数
   *
   * @param data
   * @param len
   * @param msg_id
   * @param conn
   */
  static void call_router(int msg_id, uint32_t len, const char* data,
                          NetConnection* conn);

  // construct and destruct hook function
  static void set_construct_hook(connection_callback hook,
                                 void* args = nullptr);

  static void set_destruct_hook(connection_callback hook, void* args = nullptr);

 private:
  int _sockfd;
  struct sockaddr_in _connection_addr;
  socklen_t _connection_addr_len;

  // eventLoop
  EventLoop* _loop;

 private:
  // router handler
  static message_router _router;
  // hook function
  static connection_callback _construct_hook;
  static void* _construct_hook_args;

  static connection_callback _destruct_hook;
  static void* _destruct_hook_args;

  static std::mutex _mutex;

  static int _max_conns;  // 最大client链接个数
  static std::unordered_map<int, TCPConnection*> _conns;

  // 链接池
  ThreadPool* _threadpool;
};
