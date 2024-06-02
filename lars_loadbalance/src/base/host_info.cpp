#include "base/host_info.h"

HostInfo::HostInfo(std::uint32_t ip, short port)
    : ip(ip),
      port(port),
      virtual_succ(0),
      virtual_err(0),
      real_succ(0),
      real_err(0),
      continue_succ(0),
      continue_err(0),
      overload(false) {}