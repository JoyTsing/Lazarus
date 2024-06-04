#include "lazarus/lazarus.h"

#include <netinet/in.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>

#include "arpa/inet.h"
#include "lars.pb.h"
#include "message/message.h"
#include "utils/hash.h"
#include "utils/minilog.h"

namespace lazarus {

LazarusClient::LazarusClient() : _seqid(0) {
  // 1. initialize the server address
  sockaddr_in server_addr;
  bzero(&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  // default ip and port, api & lb agent should be in the same machine
  inet_aton("127.0.0.1", &server_addr.sin_addr);
  // 2. create the socket
  for (int i = 0; i < 3; i++) {
    int fd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_UDP);
    if (fd == -1) {
      minilog::log_fatal("socket create error");
      exit(1);
    }
    _sockfd.push_back(fd);
    server_addr.sin_port = htons(8888 + i);
    // 3. connection
    if (connect(fd, (sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
      minilog::log_fatal("socket connect error");
      exit(1);
    }
  }
}

LazarusClient::~LazarusClient() {
  // 1. close the socket
  for (int fd : _sockfd) {
    close(fd);
  }
}
int LazarusClient::get_routers(int modid, int cmdid, router_set& routers) {
  char write_buf[4096], read_buf[20 * 4096];
  // 1. serialize the request
  lars::GetRouterRequest request;
  request.set_modid(modid);
  request.set_cmdid(cmdid);
  // 1.2 message head
  message_head head;
  head.message_len = request.ByteSizeLong();
  head.message_id = lars::ID_API_GetRouterRequest;
  memcpy(write_buf, &head, MESSAGE_HEAD_LEN);
  request.SerializeToArray(write_buf + MESSAGE_HEAD_LEN, head.message_len);
  // 2. send the request
  int index = hash_index(modid, cmdid);
  if (sendto(_sockfd[index], write_buf, head.message_len + MESSAGE_HEAD_LEN, 0,
             nullptr, 0) == -1) {
    minilog::log_error("sendto error");
    return lars::RET_SYSTEM_ERR;
  }
  // 3. block and wait for the response
  int message_len;
  lars::GetRouterResponse response;

  message_len =
      recvfrom(_sockfd[index], read_buf, sizeof(read_buf), 0, nullptr, 0);
  if (message_len == -1) {
    minilog::log_error("recvfrom error");
    return lars::RET_SYSTEM_ERR;
  }
  // 4. handle the response
  // 4.1 message head
  memcpy(&head, read_buf, MESSAGE_HEAD_LEN);
  if (head.message_id != lars::ID_API_GetRouterResponse) {
    minilog::log_error("message id error");
    return lars::RET_SYSTEM_ERR;
  }
  // 4.2 message body
  if (!response.ParseFromArray(read_buf + MESSAGE_HEAD_LEN,
                               message_len - MESSAGE_HEAD_LEN)) {
    minilog::log_error("parse message body error");
    return lars::RET_SYSTEM_ERR;
  }

  if (response.modid() != modid || response.cmdid() != cmdid) {
    minilog::log_error("response data error");
    return lars::RET_SYSTEM_ERR;
  }
  // 5. get the routers
  for (int i = 0; i < response.hosts_size(); i++) {
    lars::HostInfo host = response.hosts(i);
    in_addr addr;
    std::uint32_t host_ip = ntohl(host.ip());
    addr.s_addr = host_ip;
    routers.push_back(std::make_pair(inet_ntoa(addr), host.port()));
  }
  return lars::RET_SUCC;
}

int LazarusClient::get_host(int modid, int cmdid, std::string& ip,
                            short& port) {
  std::uint32_t seqid = _seqid++;
  char write_buf[4096], read_buf[20 * 4096];
  // 1. serialize the request
  lars::GetHostRequest request;
  request.set_seq(seqid);
  request.set_modid(modid);
  request.set_cmdid(cmdid);
  // 1.2 message head
  message_head head;
  head.message_len = request.ByteSizeLong();
  head.message_id = lars::ID_GetHostRequest;
  memcpy(write_buf, &head, MESSAGE_HEAD_LEN);
  request.SerializeToArray(write_buf + MESSAGE_HEAD_LEN, head.message_len);
  // 2. send the request
  int index = hash_index(modid, cmdid);
  if (sendto(_sockfd[index], write_buf, head.message_len + MESSAGE_HEAD_LEN, 0,
             nullptr, 0) == -1) {
    minilog::log_error("sendto error");
    return lars::RET_SYSTEM_ERR;
  }
  // 3. block and wait for the response
  int message_len;
  lars::GetHostResponse response;
  // TODO  maybe need to set the timeout or have better solution
  do {
    message_len =
        recvfrom(_sockfd[index], read_buf, sizeof(read_buf), 0, nullptr, 0);
    if (message_len == -1) {
      minilog::log_error("recvfrom error");
      return lars::RET_SYSTEM_ERR;
    }
    // 4. handle the response
    // 4.1 message head
    memcpy(&head, read_buf, MESSAGE_HEAD_LEN);
    if (head.message_id != lars::ID_GetHostResponse) {
      minilog::log_error("message id error");
      return lars::RET_SYSTEM_ERR;
    }
    // 4.2 message body
    if (!response.ParseFromArray(read_buf + MESSAGE_HEAD_LEN,
                                 message_len - MESSAGE_HEAD_LEN)) {
      minilog::log_error("parse message body error");
      return lars::RET_SYSTEM_ERR;
    }
  } while (response.seq() < seqid);  // handle package loss
  // 4.3 get the ip and port
  if (response.seq() != seqid || response.modid() != modid ||
      response.cmdid() != cmdid) {
    minilog::log_error("response data error");
    return lars::RET_SYSTEM_ERR;
  }

  if (response.retcode() == lars::RET_SUCC) {
    lars::HostInfo host = response.hosts();
    in_addr addr;
    std::uint32_t host_ip = ntohl(host.ip());
    addr.s_addr = host_ip;
    ip = inet_ntoa(addr);
    port = host.port();
  }

  return response.retcode();
}

void LazarusClient::report(int modid, int cmdid, std::string_view ip,
                           short port, int retcode) {
  // 1. serialize the request
  lars::ReportRequest request;
  request.set_modid(modid);
  request.set_cmdid(cmdid);
  request.set_retcode(retcode);

  auto host = request.mutable_host();
  in_addr addr;
  inet_aton(ip.data(), &addr);
  int ip_int = htonl(addr.s_addr);
  host->set_ip(ip_int);
  host->set_port(port);
  // 2. send the request
  char write_buf[4096];
  message_head head;
  head.message_len = request.ByteSizeLong();
  head.message_id = lars::ID_ReportRequest;
  memcpy(write_buf, &head, MESSAGE_HEAD_LEN);
  // 2.2 message body
  request.SerializeToArray(write_buf + MESSAGE_HEAD_LEN, head.message_len);
  // 3. send the request
  int index = hash_index(modid, cmdid);
  if (sendto(_sockfd[index], write_buf, head.message_len + MESSAGE_HEAD_LEN, 0,
             nullptr, 0) == -1) {
    minilog::log_error("sendto error");
  }
}

}  // namespace lazarus