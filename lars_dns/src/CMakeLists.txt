set(protobuf_MODULE_COMPATIBLE true)

find_package(Protobuf CONFIG REQUIRED)

# 消除absl的报错要靠grpc，无语了
find_package(gRPC CONFIG REQUIRED)
link_libraries(gRPC::grpc++)
message(STATUS "Using Protobuf ${Protobuf_VERSION}")

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

# 执行需要的依赖
add_custom_target(
  PROTO_DEP
  DEPENDS ${GENERATED_PROTOBUF_FILES}
)

include_directories(${GENERATED_PROTOBUF_PATH})

configure_file(${PROJECT_SOURCE_DIR}/conf/lars_dns.conf ./conf/lars_dns.conf COPYONLY)

file(GLOB_RECURSE all_src CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_executable(dns_server ${all_src} ${GENERATED_PROTOBUF_FILES})
add_dependencies(dns_server PROTO_DEP)

# mysql以及上一个模块reactor
target_link_libraries(dns_server PUBLIC protobuf::libprotobuf ${LARS_REACTOR_LIB} mysqlclient
  Backward::Backward Backward::Object)