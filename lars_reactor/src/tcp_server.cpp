#include "tcp_server.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/socket.h>

#include <cerrno>
#include <csignal>
#include <cstring>
#include <iostream>

#include "reactor_buf.h"

TcpServer::TcpServer(const char *ip, std::uint16_t port) {
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
        break;
      } else if (errno == EMFILE) {
        // 链接过多了
        std::cerr << "tcp::server accept() error EMFILE\n";
      } else {
        std::cerr << "tcp::server accept() error\n";
        exit(1);
      }
    } else {
      int read_len = 0;
      InputBuffer input_buffer;
      OutputBuffer output_buffer;
      char *message;
      int message_len;
      // echo
      do {
        // 客户端数据读到InputBuffer
        read_len = input_buffer.read_data(connection_fd);
        if (read_len == -1) {
          std::cerr << "TcpServer read_data() error\n";
          break;
        }
        std::cout << "read_len = " << read_len << "\n";
        message_len = input_buffer.length();
        message = new char[message_len];
        bzero(message, message_len);
        memcpy(message, input_buffer.data(), message_len);
        //  buffer已读取的数据剔除
        input_buffer.pop(message_len);
        input_buffer.adjust();
        std::cout << "server recv data = " << message << "\n ";
        // 数据写到OutputBuffer
        output_buffer.add_data(message, message_len);
        while (output_buffer.length()) {
          int written = output_buffer.write_to(connection_fd);
          if (written == -1) {
            std::cerr << " TcpServer write_to() error\n";
          } else if (written == 0) {
            // fd不可写,等待下次写
            continue;
          }
        }
        delete[] message;
      } while (read_len != 0);
      close(connection_fd);
    }
  }
}
