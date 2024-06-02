#include "balance/route_balance.h"

RouterBalance::RouterBalance(int id) : _id(id) {}

int RouterBalance::get_host(int modid, int cmdid,
                            lars::GetHostResponse& response) {
  int ret = lars::RET_SUCC;
  return ret;
}