#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace lazarus {
using ip_port = std::pair<std::string, short>;
using router_set = std::vector<ip_port>;
class LazarusClient {
 public:
  LazarusClient();
  ~LazarusClient();

  /**
   * @brief Get the host object, should use subscribe first.
   *
   * @param modid
   * @param cmdid
   * @param ip
   * @param port
   * @return int lars::ReturnCode
   */
  int get_host(int modid, int cmdid, std::string& ip, short& port);

  int get_routers(int modid, int cmdid, router_set& routers);

  /**
   * @brief report the calls status
   *
   * @param modid
   * @param cmdid
   * @param ip
   * @param retcode lars::ReturnCode
   */
  void report(int modid, int cmdid, std::string_view ip, short port,
              int retcode);

  /**
   * @brief Subscribe to the corresponding channel
   *
   * @param modid
   * @param cmdid
   * @return int lars::ReturnCode
   */
  int subscribe(int modid, int cmdid);

 private:
  std::uint32_t _seqid;      // sequence id
  std::vector<int> _sockfd;  // udp server
};

}  // namespace lazarus