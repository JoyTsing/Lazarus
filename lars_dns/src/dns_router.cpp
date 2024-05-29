#include "dns_router.h"

#include <mysql/mysql.h>

#include <cstdint>
#include <mutex>
#include <string>
#include <string_view>

#include "utils/config_file.h"
#include "utils/minilog.h"

Router::Router() {
  connect_db();
  minilog::log_info("Router init");
  // init router map from db
  load_router_map();
}

void Router::load_router_map() {
  // query from db
  // select * from RouteData;
  std::string_view query_sql = "select * from RouteData;";
  if (mysql_real_query(&_db_connection, query_sql.data(), query_sql.size()) !=
      0) {
    minilog::log_fatal("mysql_real_query failed: {}",
                       mysql_error(&_db_connection));
    exit(1);
  }
  // get result
  MYSQL_RES* result = mysql_store_result(&_db_connection);
  std::uint64_t num_rows = mysql_num_rows(result);
  MYSQL_ROW row;
  for (int i = 0; i < num_rows; i++) {
    row = mysql_fetch_row(result);
    // parse row
    int modid = std::atoi(row[1]);
    int cmdid = std::atoi(row[2]);
    unsigned int ip = std::atoi(row[3]);
    int port = std::atoi(row[4]);
    minilog::log_info("modid:{}, cmdid:{}, ip:{}, port:{}", modid, cmdid, ip,
                      port);
    // 加入到router-map中
    std::uint64_t key = ((std::uint64_t)modid << 32) + cmdid;
    std::uint64_t value = ((std::uint64_t)ip << 32) + port;
    _router_map[key].insert(value);
  }
  // TODO update _router_map
}

void Router::connect_db() {
  // 从配置文件中读取数据库连接信息
  std::string db_host =
      config_file::instance()->GetString("mysql", "db_host", "127.0.0.1");
  short db_port = config_file::instance()->GetNumber("mysql", "db_port", 3306);
  std::string db_user =
      config_file::instance()->GetString("mysql", "db_user", "root");
  std::string db_passwd =
      config_file::instance()->GetString("mysql", "db_passwd", "123456");
  std::string db_name =
      config_file::instance()->GetString("mysql", "db_name", "lars_dns");
  // init
  mysql_init(&_db_connection);
  // reconnection
  mysql_options(&_db_connection, MYSQL_OPT_CONNECT_TIMEOUT, "30");
  // WARNING: MYSQL_OPT_RECONNECT is deprecated and will be removed in a
  // future version.
  bool reconnect = true;
  mysql_options(&_db_connection, MYSQL_OPT_RECONNECT, &reconnect);
  // connect
  if (!mysql_real_connect(&_db_connection, db_host.c_str(), db_user.c_str(),
                          db_passwd.c_str(), db_name.c_str(), db_port, NULL,
                          0)) {
    minilog::log_error("mysql_real_connect failed: {}",
                       mysql_error(&_db_connection));
    exit(1);
  }
  // minilog::log_info("connect to mysql success");
}

host_set Router::get_hosts(int modid, int cmdid) {
  host_set hosts;
  std::uint64_t key = ((std::uint64_t)modid << 32) + cmdid;

  std::lock_guard<std::mutex> lock(_mutex);
  router_map_iter iter = _router_map.find(key);
  if (iter != _router_map.end()) {
    hosts = iter->second;
  }
  return hosts;
}