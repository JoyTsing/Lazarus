include_directories(${LARS_REACTOR_INCLUDE})

# PROTO
add_custom_command(
  OUTPUT
  ${LARS_H_FILE}
  ${LARS_CPP_FILE}
  COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
  ARGS
  --proto_path=${PROTO_PATH}
  --cpp_out=${GENERATED_PROTOBUF_PATH}
  ${LARS_PROTO}
  COMMENT "Running C++ Protobuf compiler..."
)

# taget
add_custom_target(
  PROTO_DEP
  DEPENDS ${GENERATED_PROTOBUF_FILES}
)

file(GLOB_RECURSE all_src CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_library(lazarusclient STATIC ${all_src})
add_dependencies(lazarusclient PROTO_DEP)

# mysql以及上一个模块reactor
target_link_libraries(lazarusclient PUBLIC protobuf::libprotobuf dl ${LARS_REACTOR_LIB} Backward::Backward Backward::Object)