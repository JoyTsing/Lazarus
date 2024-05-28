#pragma once

#include <mysql/mysql.h>

/**
 * @brief modid(模块id)/cmdid(服务id) -> host:ip,host:port的对应关系
 *
 */
class Router {
 public:
  static Router* instance() {
    static Router* instance = new Router();
    return instance;
  }

 private:
  Router();
  Router(const Router&) = delete;
  const Router& operator=(const Router&) = delete;

  /**
   * @brief 链接mysql数据库
   *
   */
  void connect_db();

 private:
  MYSQL _db_connection;
};