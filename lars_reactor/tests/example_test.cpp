#include "utils/minilog.h"
#define ANKERL_NANOBENCH_IMPLEMENT
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include <doctest.h>
#include <nanobench.h>

// NOLINTNEXTLINE
TEST_CASE("thread-pool test1") {
  minilog::set_log_level(minilog::log_level::warn);
  int iter = 0;
  // 100 times faster than mine, LOL
  ankerl::nanobench::Bench().minEpochIterations(10).run(
      "[test1] boost::asio::thread_pool speed test", [&]() {
        int res = 0;
        for (int i = 0; i < 10000; i++) {
          res += i;
        }
        minilog::log_warn("epoch {} done, add sum is {}", iter++, res);
      });
}