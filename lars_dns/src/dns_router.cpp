#include "dns_router.h"

#include <mysql/mysql.h>

#include <cstdint>
#include <cstdlib>
#include <format>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>

#include "utils/config_file.h"
#include "utils/minilog.h"

Router::Router() : _version(0) {
  connect_db();
  // init router map from db
  load_router_map(false);
  minilog::log_info("Router init done");
}

bool Router::check_version() {
  // query from db
  // select * from Version;
  std::string_view query_sql = "SELECT version FROM RouteVersion WHERE id=1;";
  if (mysql_real_query(&_db_connection, query_sql.data(), query_sql.size()) !=
      0) {
    minilog::log_fatal("mysql_real_query failed: {}",
                       mysql_error(&_db_connection));
    exit(1);
  }
  // get result

  auto result = std::shared_ptr<MYSQL_RES>(mysql_store_result(&_db_connection),
                                           mysql_free_result);
  std::uint64_t num_rows = mysql_num_rows(result.get());
  if (num_rows != 1) {
    minilog::log_fatal("No version in RouteVersion table :%s",
                       mysql_error(&_db_connection));
    exit(1);
  }
  MYSQL_ROW row = mysql_fetch_row(result.get());
  // parse row
  std::uint64_t version = std::atoll(row[0]);
  if (version != _version) {
    // minilog::log_info("version update: from old@{} to new@{}", _version,
    //                   (std::uint64_t)version);
    _version = version;
    return true;
  }
  return false;
}

void Router::load_changes(std::vector<std::uint64_t>& change_list) {
  // query from db
  std::string query_sql = std::format(
      "SELECT modid,cmdid from RouteChange WHERE version>= {};", _version);
  if (mysql_real_query(&_db_connection, query_sql.data(), query_sql.size()) !=
      0) {
    minilog::log_fatal("mysql_real_query failed: {}",
                       mysql_error(&_db_connection));
    exit(1);
  }

  // get result
  auto result = std::shared_ptr<MYSQL_RES>(mysql_store_result(&_db_connection),
                                           mysql_free_result);
  std::uint64_t num_rows = mysql_num_rows(result.get());
  if (num_rows == 0) {
    // minilog::log_info("No change in RouteChange table");
    return;
  }
  // parse rows
  MYSQL_ROW row;
  for (int i = 0; i < num_rows; i++) {
    row = mysql_fetch_row(result.get());
    // parse row
    int modid = std::atoi(row[0]);
    int cmdid = std::atoi(row[1]);
    minilog::log_info("[change mods] @{}: modid:{}, cmdid:{}", i, modid, cmdid);
    // 加入到change_list中
    std::uint64_t key = ((std::uint64_t)modid << 32) + cmdid;
    change_list.push_back(key);
  }
}

void Router::load_router_map(bool is_bak) {
  // query from db
  // select * from RouteData;
  std::string_view query_sql = "SELECT * from RouteData;";
  if (mysql_real_query(&_db_connection, query_sql.data(), query_sql.size()) !=
      0) {
    minilog::log_fatal("mysql_real_query failed: {}",
                       mysql_error(&_db_connection));
    exit(1);
  }
  // get result
  auto result = std::shared_ptr<MYSQL_RES>(mysql_store_result(&_db_connection),
                                           mysql_free_result);
  std::uint64_t num_rows = mysql_num_rows(result.get());
  MYSQL_ROW row;
  // clear _router_map_bak
  _router_map_bak.clear();
  // get rows
  for (int i = 0; i < num_rows; i++) {
    row = mysql_fetch_row(result.get());
    // parse row
    int modid = std::atoi(row[1]);
    int cmdid = std::atoi(row[2]);
    unsigned int ip = std::atoi(row[3]);
    int port = std::atoi(row[4]);
    // minilog::log_info("modid:{}, cmdid:{}, ip:{}, port:{}", modid, cmdid, ip,
    //                   port);
    // 加入到router-map中
    std::uint64_t key = ((std::uint64_t)modid << 32) + cmdid;
    std::uint64_t value = ((std::uint64_t)ip << 32) + port;
    if (is_bak) {
      _router_map_bak[key].insert(value);
    } else {
      _router_map[key].insert(value);
    }
  }
}

void Router::update_router_map() {
  std::lock_guard<std::mutex> lock(_mutex);
  std::swap(_router_map, _router_map_bak);
}

void Router::remove_changes(bool remove_all) {
  std::string query_sql;
  if (remove_all) {
    query_sql = "DELETE FROM RouteChange;";
  } else {
    query_sql =
        std::format("DELETE FROM RouteChange WHERE version <= {}", _version);
  }
  if (mysql_real_query(&_db_connection, query_sql.data(), query_sql.size()) !=
      0) {
    minilog::log_fatal("mysql_real_query failed: {}",
                       mysql_error(&_db_connection));
    exit(1);
  }
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