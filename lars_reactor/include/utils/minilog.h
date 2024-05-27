#pragma once

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <fstream>
#include <iostream>
#include <source_location>
#include <string_view>

namespace minilog {

#define MINILOG_FOREACH_LOG_LEVEL(f) \
  f(trace) f(debug) f(info) f(critical) f(warn) f(error) f(fatal)

enum class log_level : std::uint8_t {
#define _FUNCTION(name) name,
  MINILOG_FOREACH_LOG_LEVEL(_FUNCTION)
#undef _FUNCTION
};

namespace details {
// 上色
#if defined(__linux__) || defined(__APPLE__)
inline constexpr char
    k_level_ansi_colors[(std::uint8_t)log_level::fatal + 1][8] = {
        "\E[37m", "\E[35m", "\E[32m", "\E[34m", "\E[33m", "\E[31m", "\E[31;1m",
};
inline constexpr char k_reset_ansi_color[4] = "\E[m";
#define _MINILOG_IF_HAS_ANSI_COLORS(x) x
#else
#define _MINILOG_IF_HAS_ANSI_COLORS(x)
inline constexpr char k_level_ansi_colors[(std::uint8_t)log_level::fatal + 1]
                                         [1] = {
                                             "", "", "", "", "", "", "",
};
inline constexpr char k_reset_ansi_color[1] = "";
#endif

inline std::string log_level_name(log_level level) {
  switch (level) {
#define _FUNCTION(name) \
  case log_level::name: \
    return #name;
    MINILOG_FOREACH_LOG_LEVEL(_FUNCTION)
#undef _FUNCTION
  }
  return "unknown";
}

inline log_level log_level_from_name(std::string_view lev) {
#define _FUNCTION(name) \
  if (lev == #name) return log_level::name;
  MINILOG_FOREACH_LOG_LEVEL(_FUNCTION)
#undef _FUNCTION
  return log_level::info;
}

// 支持export中获取log的等级
inline log_level g_max_level = []() -> log_level {
  if (auto lev = std::getenv("MINILOG_LEVEL"); lev) {
    return details::log_level_from_name(lev);
  }
  return log_level::info;
}();

// 要输出到的文件
inline std::ofstream g_log_file = []() -> std::ofstream {
  if (auto path = std::getenv("MINILOG_FILE"); path) {
    return std::ofstream(path);
  }
  return std::ofstream();
}();

inline void output_log(log_level level, std::string msg,
                       std::source_location const &loc) {
  std::chrono::zoned_time now{std::chrono::current_zone(),
                              std::chrono::high_resolution_clock::now()};
  msg = std::format("{} {}:{} [{}] {}", now, loc.file_name(), loc.line(),
                    details::log_level_name(level), msg);
  if (g_log_file.is_open()) {
    g_log_file << msg << std::endl;
  }
  if (level >= g_max_level) {
    std::cout << _MINILOG_IF_HAS_ANSI_COLORS(
                     k_level_ansi_colors[(std::uint8_t)level] +)
                         msg _MINILOG_IF_HAS_ANSI_COLORS(+k_reset_ansi_color) +
                     '\n';
  }
}

template <class T>
struct with_source_location {
 private:
  T inner_;
  std::source_location loc_;

 public:
  template <class U>
    requires std::constructible_from<T, U>
  // 首先，这是一个模板函数，它接受一个类型为 U 的参数。这个
  // U 类型必须满足 std::constructible_from<T, U>
  // 的要求，也就是说，类型 T 必须能够从类型 U 构造。
  consteval with_source_location(  // 构造一个带有源代码位置信息的对象，consteval
                                   // 环境中定义的，这意味着它在编译时就会被执行。
      U &&inner, std::source_location loc = std::source_location::current())
      : inner_(std::forward<U>(inner)), loc_(loc) {}
  constexpr T const &format() const { return inner_; }
  constexpr const std::source_location &location() const { return loc_; }
};
}  // namespace details

inline void set_log_file(std::string path) {
  details::g_log_file = std::ofstream(path, std::ios::app);
}

inline void set_log_level(log_level lev) { details::g_max_level = lev; }

template <typename... Args>
void generic_log(log_level level,
                 details::with_source_location<std::format_string<Args...>> fmt,
                 Args &&...args) {
  auto const &loc = fmt.location();
  auto msg = std::vformat(fmt.format().get(), std::make_format_args(args...));
  details::output_log(level, std::move(msg), loc);
}

#define _FUNCTION(name)                                                        \
  template <typename... Args>                                                  \
  void log_##name(                                                             \
      details::with_source_location<std::format_string<Args...>> fmt,          \
      Args &&...args) {                                                        \
    generic_log(log_level::name, std::move(fmt), std::forward<Args>(args)...); \
  }
MINILOG_FOREACH_LOG_LEVEL(_FUNCTION)
#undef _FUNCTION

#define MINILOG_PRINT_VAR(x) ::minilog::log_debug(#x "={}", x)

}  // namespace minilog