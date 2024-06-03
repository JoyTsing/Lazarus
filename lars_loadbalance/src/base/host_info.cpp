#include "base/host_info.h"

#include "base/init.h"

HostInfo::HostInfo(std::uint32_t ip, short port, int init_success)
    : ip(ip),
      port(port),
      virtual_succ(init_success),
      virtual_err(0),
      real_succ(0),
      real_err(0),
      continue_succ(0),
      continue_err(0),
      overload(false) {}

void HostInfo::set_idle() {
  virtual_succ = 0;
  virtual_err = loadbalance::base::lb_config.init_error_cnt;
  real_succ = 0;
  real_err = 0;
  continue_succ = 0;
  continue_err = 0;
  overload = true;
}

void HostInfo::set_overload() {
  virtual_succ = loadbalance::base::lb_config.init_success_cnt;
  virtual_err = 0;
  real_succ = 0;
  real_err = 0;
  continue_succ = 0;
  continue_err = 0;
  overload = false;
}