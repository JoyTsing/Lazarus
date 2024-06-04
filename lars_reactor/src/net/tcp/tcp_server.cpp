#include "net/tcp/tcp_server.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include <cerrno>
#include <csignal>
#include <cstring>
#include <mutex>

#include "message/message.h"
#include "message/task_message.h"
#include "net/net_connection.h"
#include "net/tcp/tcp_connection.h"
#include "utils/config_file.h"
#include "utils/minilog.h"
#include "utils/thread_queue.h"

// using io_call_back = std::function<void(EventLoop* el, int fd, void* args)>;

// static source init
int TcpServer::_max_conns = 0;
std::mutex TcpServer::_mutex;
std::unordered_map<int, TCPConnection *> TcpServer::_conns;
message_router TcpServer::_router;

connection_callback TcpServer::_construct_hook = nullptr;
void *TcpServer::_construct_hook_args = nullptr;

connection_callback TcpServer::_destruct_hook = nullptr;
void *TcpServer::_destruct_hook_args = nullptr;

TcpServer::TcpServer(EventLoop *loop, const char *ip, std::uint16_t port)
    : _loop(loop) {
  bzero(&_connection_addr, sizeof(_connection_addr));
  _connection_addr_len = sizeof(_connection_addr);
  // 忽略一些信号 SIGHUP, SIGPIPE
  // SIGPIPE:如果客户端关闭，服务端再次write就会产生
  // SIGHUP:如果terminal关闭，会给当前进程发送该信号
  if (signal(SIGHUP, SIG_IGN) == SIG_ERR) {
    minilog::log_error("signal ignore SIGHUP error");
  }

  if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
    minilog::log_error("signal ignore SIGPIPE error");
  }

  // Create a socket
  _sockfd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, IPPROTO_TCP);
  // Check if the socket was created successfully
  if (_sockfd == -1) {
    minilog::log_error("tcp::server socket() error");
    exit(1);
  }
  // init connection address
  struct sockaddr_in server_addr;
  bzero(&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  inet_aton(ip, &server_addr.sin_addr);
  server_addr.sin_port = htons(port);

  // set socket CAN REUSE
  int op = 1;
  if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &op, sizeof(op)) < 0) {
    minilog::log_error("tcp::server setsockopt() error");
    exit(1);
  }

  // bind the port
  if (bind(_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    minilog::log_error("tcp::server bind() error");
    exit(1);
  }
  // listen
  if (listen(_sockfd, 1024) == -1) {
    minilog::log_error("tcp::server listen() error");
    exit(1);
  }

  // init connection number
  _max_conns =
      config_file::instance()->GetNumber("reactor", "maxConnection", 20000);
  // init thread pool
  int thread_num =
      config_file::instance()->GetNumber("reactor", "threadNum", 20);
  _threadpool = new ThreadPool(thread_num);
  // 注册socket读事件accept到eventLoop
  _loop->add_io_event(
      _sockfd, EPOLLIN,
      [](IO_EVENT_ARGUMENT) {
        TcpServer *server = (TcpServer *)args;
        server->handle_accept();
      },
      this);
  minilog::log_info("tcp::server start...");
}

TcpServer::~TcpServer() { close(_sockfd); }

void TcpServer::handle_accept() {
  int connection_fd;
  while (true) {
    // 1 accept
    connection_fd = accept(_sockfd, (struct sockaddr *)&_connection_addr,
                           &_connection_addr_len);
    if (connection_fd == -1) {
      if (errno == EINTR) {
        // 如果是中断信号，继续accept
        continue;
      } else if (errno == EAGAIN) {
        // 如果是非阻塞模式，且没有连接，返回EAGAIN
        minilog::log_error("tcp::server accept() error EAGAIN");
        break;
      } else if (errno == EMFILE) {
        // 链接过多了
        minilog::log_error("tcp::server accept() error EMFILE");
        continue;
      } else {
        minilog::log_error("tcp::server accept() error");
        exit(1);
      }
    }
    // accept success
    if (get_connection_num() >= _max_conns) {
      minilog::log_error("tcp::server connection num is max");
      close(connection_fd);
    } else {
      if (_threadpool != nullptr) {
        // 获取thread_queue
        auto queue = _threadpool->get_thread_queue();
        task_message task;
        task.type = task_message::TaskType::NEW_CONNECTION;
        task.data = connection_fd;
        // conn_fd 传递给线程池
        queue->send(task);
      } else {
        new TCPConnection(connection_fd, _loop);
      }
    }
    break;
  }
}

void TcpServer::call_router(int msg_id, uint32_t len, const char *data,
                            NetConnection *conn) {
  _router.call_router(msg_id, len, data, conn);
}

void TcpServer::set_conn_start_hook(connection_callback hook, void *args) {
  _construct_hook = hook;
  _construct_hook_args = args;
}

void TcpServer::set_conn_close_hook(connection_callback hook, void *args) {
  _destruct_hook = hook;
  _destruct_hook_args = args;
}

void TcpServer::add_message_router(int msg_id, message_callback handler,
                                   void *args) {
  _router.register_router(msg_id, handler, args);
}

void TcpServer::add_connection(int conn_fd, TCPConnection *conn) {
  std::lock_guard<std::mutex> lock(_mutex);
  _conns[conn_fd] = conn;
}

void TcpServer::remove_connection(int conn_fd) {
  std::lock_guard<std::mutex> lock(_mutex);
  if (_conns.find(conn_fd) != _conns.end()) {
    // minilog::log_info("remove connection: {}", conn_fd);
    _conns.erase(conn_fd);
  }
}

int TcpServer::get_connection_num() { return _conns.size(); }

TCPConnection *TcpServer::get_connection(int conn_fd) {
  return conn_fd < 0 ? nullptr : _conns[conn_fd];
}

ThreadPool *TcpServer::get_threadpool() { return _threadpool; }