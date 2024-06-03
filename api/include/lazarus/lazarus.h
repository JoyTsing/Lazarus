#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace lazarus {

class LazarusClient {
 public:
  LazarusClient();
  ~LazarusClient();

  /**
   * @brief Get the hosts object
   *
   * @param modid
   * @param cmdid
   * @param ip
   * @param port
   * @return int lars::ReturnCode
   */
  int get_hosts(int modid, int cmdid, std::string& ip, short& port);

  /**
   * @brief report the calls status
   *
   * @param modid
   * @param cmdid
   * @param ip
   * @param retcode : lars::ReturnCode
   */
  void report(int modid, int cmdid, std::string_view ip, short port,
              int retcode);

 private:
  std::uint32_t _seqid;      // sequence id
  std::vector<int> _sockfd;  // udp server
};

}  // namespace lazarus