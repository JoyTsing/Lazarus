#pragma once
/**
 * @brief 用于获取sever的index
 *
 * @param modid
 * @param cmdid
 * @return int
 */
inline int hash_index(int modid, int cmdid) {
  // todo 得找个更好的hash算法
  return (modid + cmdid) % 3;
}
