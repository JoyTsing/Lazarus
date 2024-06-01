#pragma once

#include <mysql/mysql.h>

#include "lars.pb.h"

class ReporterStore {
 public:
  ReporterStore();

  /**
   * @brief 将ReportStatus内存储的多个节点状态信息存储到数据库中
   *
   * @param request
   */
  void store(lars::ReportStatusRequest& request);

 private:
  MYSQL _conn;
};