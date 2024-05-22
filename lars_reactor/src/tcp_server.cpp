#include "tcp_server.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include <cerrno>
#include <csignal>
#include <cstring>
#include <iostream>

#include "reactor_buf.h"

// using io_call_back = std::function<void(EventLoop* el, int fd, void* args)>;
void accept_callback(EventLoop *el, int fd, void *args) {
  TcpServer *server = (TcpServer *)args;
  server->do_accept();
}

// 测试用的首发消息结构体
struct Message {
  int len;
  char buf[4096];
};

void server_read_callback(EventLoop *el, int fd, void *args);

void server_write_callback(EventLoop *el, int fd, void *args) {
  Message *message = (Message *)args;
  // 写回数据
  OutputBuffer output_buf;
  output_buf.add_data(message->buf, message->len);
  while (output_buf.length()) {
    int write_len = output_buf.write_to(fd);
    if (write_len == -1) {
      std::cerr << "write error\n";
      return;
    } else if (write_len == 0) {
      // 不可写
      break;
    }
  }
  // 写完了，注册读事件
  el->del_io_event(fd, EPOLLOUT);
  el->add_io_event(fd, EPOLLIN, server_read_callback, message);
}

// 客户端connect成功后，注册的读回调函数
void server_read_callback(EventLoop *el, int fd, void *args) {
  Message *message = (Message *)args;
  // 读取数据
  InputBuffer input_buf;
  int read_len = input_buf.read_data(fd);
  if (read_len == -1 || read_len == 0) {
    std::cerr << "close\n";
    // 删除事件
    el->del_io_event(fd);
    close(fd);
    return;
  }
  // 读取成功
  std::cout << "server read_callback\n";
  // 将数据写回
  message->len = input_buf.length();
  bzero(message->buf, message->len);
  memcpy(message->buf, input_buf.data(), message->len);

  input_buf.pop(message->len);
  input_buf.adjust();

  std::cout << "recv data:" << message->buf << std::endl;
  // echo
  el->del_io_event(fd, EPOLLIN);
  el->add_io_event(fd, EPOLLOUT, server_write_callback, message);
}

TcpServer::TcpServer(EventLoop *loop, const char *ip, std::uint16_t port)
    : _loop(loop) {
  bzero(&_connection_addr, sizeof(_connection_addr));
  // 忽略一些信号 SIGHUP, SIGPIPE
  // SIGPIPE:如果客户端关闭，服务端再次write就会产生
  // SIGHUP:如果terminal关闭，会给当前进程发送该信号
  if (signal(SIGHUP, SIG_IGN) == SIG_ERR) {
    std::cerr << "signal ignore SIGHUP error\n";
  }

  if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
    std::cerr << "signal ignore SIGPIPE error\n";
  }

  // Create a socket
  _sockfd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, IPPROTO_TCP);
  // Check if the socket was created successfully
  if (_sockfd == -1) {
    std::cerr << "tcp::server socket() error\n";
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
  if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &op, sizeof(op))) {
    std::cerr << "tcp::server setsockopt() error\n";
    exit(1);
  }

  // bind the port
  if (bind(_sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) ==
      -1) {
    std::cerr << "tcp::server bind() error\n";
    exit(1);
  }
  // listen
  if (listen(_sockfd, 1024) == -1) {
    std::cerr << "tcp::server listen() error\n";
    exit(1);
  }
  // 注册socket读事件accept到eventLoop
  _loop->add_io_event(_sockfd, EPOLLIN, accept_callback, this);
}

TcpServer::~TcpServer() { close(_sockfd); }

void TcpServer::do_accept() {
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
        std::cerr << "tcp::server accept() error EAGAIN\n";
        break;
      } else if (errno == EMFILE) {
        // 链接过多了
        std::cerr << "tcp::server accept() error EMFILE\n";
        continue;
      } else {
        std::cerr << "tcp::server accept() error\n";
        exit(1);
      }
    }
    std::cout << "accept success\n";
    // accept success
    Message message;
    _loop->add_io_event(connection_fd, EPOLLIN, server_read_callback, &message);
    return;
  }
}
