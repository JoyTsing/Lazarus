#include "base/hash.h"

int loadbalance::hash_index(int modid, int cmdid) {
  // todo 得找个更好的hash算法
  return (modid + cmdid) % 3;
}
