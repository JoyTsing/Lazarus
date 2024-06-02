set(protobuf_MODULE_COMPATIBLE true)

find_package(Protobuf CONFIG REQUIRED)

# 消除absl的报错要靠grpc，无语了
find_package(gRPC CONFIG REQUIRED)
link_libraries(gRPC::grpc++)

# api
set(LAZARUS_LIB ${CMAKE_BINARY_DIR}/src/liblazarusclient.a)

# 执行需要的依赖
add_custom_target(
  EXAMPLE_GEN_PROTO
  DEPENDS ${GENERATED_PROTOBUF_FILES} ${LAZARUS_LIB}
)

include_directories(${GENERATED_PROTOBUF_PATH})

file(GLOB_RECURSE all_src ${PROJECT_SOURCE_DIR}/examples/*.cpp)

foreach(v ${all_src})
  string(REGEX MATCH "examples/.*" relative_path ${v})
  string(REGEX REPLACE "examples/" "" target_name ${relative_path})
  string(REGEX REPLACE ".cpp" "" target_name ${target_name})

  add_executable(${target_name} ${v} ${GENERATED_PROTOBUF_FILES})
  target_link_libraries(${target_name} PUBLIC ${LAZARUS_LIB} protobuf::libprotobuf Backward::Backward Backward::Object)
endforeach()