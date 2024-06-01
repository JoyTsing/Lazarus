#include "reporter_task.h"

#include "lars.pb.h"
#include "reporter_store.h"

void get_reporter_status(MESSAGE_ROUTER_ARGS) {
  lars::ReportStatusRequest request;
  request.ParseFromArray(data, len);
  // request存储进数据中
  ReporterStore store;
  store.store(request);
}