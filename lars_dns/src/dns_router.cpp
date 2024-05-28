#include "dns_router.h"

#include <mysql/mysql.h>

#include "utils/config_file.h"
#include "utils/minilog.h"

Router::Router() {
  connect_db();
  minilog::log_info("Router init");
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
  // WARNING: MYSQL_OPT_RECONNECT is deprecated and will be removed in a future
  // version.
  bool reconnect = true;
  mysql_options(&_db_connection, MYSQL_OPT_RECONNECT, &reconnect);
  // connect
  if (!mysql_real_connect(&_db_connection, db_host.c_str(), db_user.c_str(),
                          db_passwd.c_str(), db_name.c_str(), db_port, NULL,
                          0)) {
    minilog::log_error("mysql_real_connect failed: %s",
                       mysql_error(&_db_connection));
    exit(1);
  }
}
