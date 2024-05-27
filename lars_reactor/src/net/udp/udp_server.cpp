#include "net/udp/udp_server.h"

#include <arpa/inet.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <cstring>

#include "message/message.h"

UdpServer::UdpServer(EventLoop* loop, const char* ip, std::uint16_t port)
    : _loop(loop) {
  bzero(&_connection_addr, sizeof(_connection_addr));
  _connection_addr_len = sizeof(_connection_addr);
  if (signal(SIGHUP, SIG_IGN) == SIG_ERR) {
    std::cerr << "signal ignore SIGHUP error\n";
  }

  if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
    std::cerr << "signal ignore SIGPIPE error\n";
  }

  _sockfd =
      socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_UDP);
  if (_sockfd == -1) {
    std::cerr << "udp::server socket() error\n";
    exit(1);
  }
  // init connection address
  struct sockaddr_in server_addr;
  bzero(&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  inet_aton(ip, &server_addr.sin_addr);
  server_addr.sin_port = htons(port);

  // bind the port
  if (bind(_sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    std::cerr << "tcp::server bind() error\n";
    exit(1);
  }

  // 注册socket读事件accept到eventLoop
  add_read_event(_sockfd);
}

UdpServer::~UdpServer() {
  _loop->del_io_event(_sockfd);
  close(_sockfd);
}

int UdpServer::send_message(const char* data, int len, int message_id) {
  if (len > MESSAGE_LENGTH_LIMIT) {
    std::cerr << "udp::server send_message() error\n";
    return -1;
  }
  // 1. 封装数据
  message_head head;
  head.message_id = message_id;
  head.message_len = len;
  memcpy(_write_buffer, &head, MESSAGE_HEAD_LEN);
  memcpy(_write_buffer + MESSAGE_HEAD_LEN, data, len);
  // 2. 发送数据
  int ret = sendto(_sockfd, _write_buffer, len + MESSAGE_HEAD_LEN, 0,
                   (struct sockaddr*)&_connection_addr, _connection_addr_len);
  if (ret == -1) {
    std::cerr << "udp::server sendto() error\n";
    return -1;
  }
  return ret;
}

void UdpServer::add_message_router(int msg_id, message_callback handler,
                                   void* args) {
  _router.register_router(msg_id, handler, args);
}

void UdpServer::handle_read() {
  while (true) {
    int package_len =
        recvfrom(_sockfd, _read_buffer, sizeof(_read_buffer), 0,
                 (struct sockaddr*)&_connection_addr, &_connection_addr_len);
    if (package_len == -1) {
      if (errno == EINTR) {
        continue;
      } else if (errno == EAGAIN) {
        // NONBLOCK模式下，数据读取完毕
        break;
      } else {
        std::cerr << "udp::server recvfrom() error\n";
        break;
      }
    }
    // 处理数据
    // 1. 解析数据
    message_head head;
    memcpy(&head, _read_buffer, MESSAGE_HEAD_LEN);
    if (head.message_len > MESSAGE_LENGTH_LIMIT || head.message_len < 0 ||
        head.message_len + MESSAGE_HEAD_LEN != package_len) {
      std::cerr << "udp::server message length error\n";
      continue;
    }

    // 2. 调用注册的回调函数
    _router.call_router(head.message_id, head.message_len,
                        _read_buffer + MESSAGE_HEAD_LEN, this);
  }
}

void UdpServer::add_read_event(int fd) {
  _loop->add_io_event(
      fd, EPOLLIN,
      [](IO_EVENT_ARGUMENT) {
        UdpServer* server = (UdpServer*)args;
        server->handle_read();
      },
      this);
}
