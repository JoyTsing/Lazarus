#pragma once

#include <mysql/mysql.h>

#include "lars.pb.h"

class ReporterStore {
 public:
  /**
   * @brief 将ReportStatus内存储的多个节点状态信息存储到数据库中
   *
   * @param request
   */
  void store(const lars::ReportStatusRequest& request);
  ReporterStore();

  // 提高性能，单例模式仍然只有一个对象操作数据库，理想情况下每个线程都有一个数据库连接
  // static std::shared_ptr<ReporterStore> instance() {
  //   static auto _instance = std::shared_ptr<ReporterStore>(new
  //   ReporterStore); return _instance;
  // }

 private:
  MYSQL _conn;
};