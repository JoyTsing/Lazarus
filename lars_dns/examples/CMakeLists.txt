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

set(LARS_CPP_FILE ${GENERATED_PROTOBUF_PATH}/lars.pb.cc)
set(LARS_H_FILE ${GENERATED_PROTOBUF_PATH}/lars.pb.h)

set(GENERATED_PROTOBUF_FILES
  ${LARS_H_FILE}
  ${LARS_CPP_FILE}
)

include_directories(${GENERATED_PROTOBUF_PATH})

configure_file(${PROJECT_SOURCE_DIR}/conf/lars_dns.conf ./conf/lars_dns.conf COPYONLY)

file(GLOB_RECURSE all_src ${PROJECT_SOURCE_DIR}/examples/*.cpp)

foreach(v ${all_src})
  string(REGEX MATCH "examples/.*" relative_path ${v})
  string(REGEX REPLACE "examples/" "" target_name ${relative_path})
  string(REGEX REPLACE ".cpp" "" target_name ${target_name})

  add_executable(${target_name} ${v} ${GENERATED_PROTOBUF_FILES})
  target_link_libraries(${target_name} PUBLIC protobuf::libprotobuf ${LARS_REACTOR_LIB} mysqlclient)
endforeach()