#pragma once

#include <cstdint>

/**
 * @brief 一个主机的基本信息
 *
 */
class HostInfo {
 public:
  HostInfo(std::uint32_t ip, short port);

  std::uint32_t ip;  // host被代理主机IP
  int port;          // host被代理主机端口

  std::uint32_t
      virtual_succ;  // 虚拟成功次数(API汇报节点调用结果是成功时)，用于过载(overload)，空闲(idle)判定
  std::uint32_t
      virtual_err;  // 虚拟失败个数(API汇报节点调用结果是失败时)，用于过载(overload)，空闲(idle)判定
  std::uint32_t real_succ;  // 真实运行成功个数, 给Reporter上报用户观察
  std::uint32_t real_err;   // 真实运行失败个数，给Reporter上报用户观察

  std::uint32_t continue_succ;  // 连续成功次数
  std::uint32_t continue_err;   // 连续失败次数

  bool overload;  // 是否过载
};