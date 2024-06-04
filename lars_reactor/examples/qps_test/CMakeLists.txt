# for each "src/x.cpp", generate target "x"
# file(GLOB_RECURSE all_srcs CONFIGURE_DEPENDS *.cpp)
# set(Protobuf_PREFIX_PATH
# "/home/joytsing/makeInstall/protobuf3.21.12/include"
# "/home/joytsing/makeInstall/protobuf3.21.12/lib"
# "/home/joytsing/makeInstall/protobuf3.21.12/bin")

# list(APPEND CMAKE_PREFIX_PATH "${Protobuf_PREFIX_PATH}")

set(protobuf_MODULE_COMPATIBLE true)

find_package(Protobuf CONFIG REQUIRED)

# TODO:消除absl的报错要靠grpc，无语了
find_package(gRPC CONFIG REQUIRED)
link_libraries(gRPC::grpc++)
message(STATUS "Using Protobuf ${Protobuf_VERSION}")

# 生成pb文件
set(PROTO_PATH ${CMAKE_CURRENT_SOURCE_DIR}/proto)
set(ECHOMESSAGE_PROTO ${PROTO_PATH}/echoMessage.proto)
set(GENERATED_PROTOBUF_PATH ${CMAKE_BINARY_DIR}/proto)
file(MAKE_DIRECTORY ${GENERATED_PROTOBUF_PATH})

set(ECHOMESSAGE_PB_CPP_FILE ${GENERATED_PROTOBUF_PATH}/echoMessage.pb.cc)
set(ECHOMESSAGE_PB_H_FILE ${GENERATED_PROTOBUF_PATH}/echoMessage.pb.h)

add_custom_command(
  OUTPUT
  ${ECHOMESSAGE_PB_H_FILE}
  ${ECHOMESSAGE_PB_CPP_FILE}
  COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
  ARGS
  --proto_path=${PROTO_PATH}
  --cpp_out=${GENERATED_PROTOBUF_PATH}
  ${ECHOMESSAGE_PROTO}
  COMMENT "Running C++ protocol buffer compiler"
)

set(GENERATED_PROTOBUF_FILES
  ${ECHOMESSAGE_PB_H_FILE}
  ${ECHOMESSAGE_PB_CPP_FILE}
)

include_directories(${GENERATED_PROTOBUF_PATH})

file(GLOB_RECURSE all_tests *.cpp)

configure_file(${PROJECT_SOURCE_DIR}/conf/server.conf ./conf/server.conf COPYONLY)

foreach(v ${all_tests})
  string(REGEX MATCH "examples/qps_test/.*" relative_path ${v})
  string(REGEX REPLACE "examples/qps_test/" "" target_name ${relative_path})
  string(REGEX REPLACE ".cpp" "" target_name ${target_name})

  add_executable(${target_name} ${v} ${GENERATED_PROTOBUF_FILES})
  target_link_libraries(${target_name} PUBLIC protobuf::libprotobuf reactor Backward::Backward Backward::Object)
endforeach()