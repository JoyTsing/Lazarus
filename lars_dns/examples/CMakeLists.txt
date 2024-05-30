set(protobuf_MODULE_COMPATIBLE true)

find_package(Protobuf CONFIG REQUIRED)

# 消除absl的报错要靠grpc，无语了
find_package(gRPC CONFIG REQUIRED)
link_libraries(gRPC::grpc++)

# 执行需要的依赖
add_custom_target(
  EXAMPLE_GEN_PROTO
  DEPENDS ${GENERATED_PROTOBUF_FILES}
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