set(protobuf_MODULE_COMPATIBLE true)

find_package(Protobuf CONFIG REQUIRED)

# TODO:消除absl的报错要靠grpc，无语了
find_package(gRPC CONFIG REQUIRED)
link_libraries(gRPC::grpc++)
message(STATUS "Using Protobuf ${Protobuf_VERSION}")

# 生成pb文件
set(PROTO_PATH ${LARS_PATH}/proto)
set(LARS_PROTO ${PROTO_PATH}/lars.proto)
set(GENERATED_PROTOBUF_PATH ${CMAKE_BINARY_DIR}/proto)
file(MAKE_DIRECTORY ${GENERATED_PROTOBUF_PATH})

set(LARS_CPP_FILE ${GENERATED_PROTOBUF_PATH}/lars.pb.cc)
set(LARS_H_FILE ${GENERATED_PROTOBUF_PATH}/lars.pb.h)

add_custom_command(
  OUTPUT
  ${LARS_H_FILE}
  ${LARS_CPP_FILE}
  COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
  ARGS
  --proto_path=${PROTO_PATH}
  --cpp_out=${GENERATED_PROTOBUF_PATH}
  ${LARS_PROTO}
  COMMENT "Running C++ protocol buffer compiler"
)

set(GENERATED_PROTOBUF_FILES
  ${LARS_H_FILE}
  ${LARS_CPP_FILE}
)

include_directories(${GENERATED_PROTOBUF_PATH})

configure_file(${PROJECT_SOURCE_DIR}/conf/lars_dns.conf ./conf/lars_dns.conf COPYONLY)

file(GLOB_RECURSE all_src *.cpp)
add_executable(dns_service ${all_src} ${GENERATED_PROTOBUF_FILES})

# mysql以及上一个模块reactor
target_link_libraries(dns_service PUBLIC protobuf::libprotobuf ${LARS_REACTOR_LIB} mysqlclient)