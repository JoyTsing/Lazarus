#include "reporter_store.h"

#include <mysql/mysql.h>
#include <utils/config_file.h>

#include <format>

#include "lars.pb.h"
#include "utils/minilog.h"

ReporterStore::ReporterStore() {
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
  // init mysql connection
  mysql_init(&_conn);
  mysql_options(&_conn, MYSQL_OPT_CONNECT_TIMEOUT, "30");

  bool reconnect = true;
  mysql_options(&_conn, MYSQL_OPT_RECONNECT, &reconnect);
  // connect
  if (!mysql_real_connect(&_conn, db_host.c_str(), db_user.c_str(),
                          db_passwd.c_str(), db_name.c_str(), db_port, NULL,
                          0)) {
    minilog::log_error("mysql_real_connect failed: {}", mysql_error(&_conn));
    exit(1);
  }
}

void ReporterStore::store(const lars::ReportStatusRequest& request) {
  for (int i = 0; i < request.results_size(); i++) {
    const lars::HostCallResult& result = request.results(i);
    int overload = result.overload() ? 1 : 0;  // 数据库中的overload是char类型
    std::string sql = std::format(
        "INSERT INTO ServerCallStatus (modid, cmdid, ip, port, caller, "
        "succ_cnt, err_cnt, ts, overload)"
        "VALUES ({}, {}, {}, {}, {}, {}, {}, {}, {}) ON DUPLICATE KEY "
        "UPDATE succ_cnt = {}, err_cnt = {}, ts = {}, overload = {}",
        request.modid(), request.cmdid(), result.ip(), result.port(),
        request.caller(), result.succ(), result.fail(), request.timestamp(),
        overload, result.succ(), result.fail(), request.timestamp(), overload);

    if (mysql_real_query(&_conn, sql.data(), sql.size()) != 0) {
      minilog::log_error("Fail to Insert into ServerCallStatus {}",
                         mysql_error(&_conn));
      std::cout << sql << std::endl;
    }
  }
}
