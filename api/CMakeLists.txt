cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
  message("Setting default build type to Debug")
endif()

project(lars_api VERSION 0.1 LANGUAGES CXX)

get_filename_component(LARS_PATH ${PROJECT_SOURCE_DIR}/../ ABSOLUTE)

# PROTO
set(PROTO_PATH ${LARS_PATH}/proto)
set(LARS_PROTO ${PROTO_PATH}/lars.proto)
set(GENERATED_PROTOBUF_PATH ${CMAKE_BINARY_DIR}/proto)

set(LARS_CPP_FILE ${GENERATED_PROTOBUF_PATH}/lars.pb.cc)
set(LARS_H_FILE ${GENERATED_PROTOBUF_PATH}/lars.pb.h)

set(GENERATED_PROTOBUF_FILES
  ${LARS_H_FILE}
  ${LARS_CPP_FILE}
)

# PROTO LIB
set(protobuf_MODULE_COMPATIBLE true)

find_package(Protobuf CONFIG REQUIRED)

# 消除absl的报错要靠grpc，无语了
find_package(gRPC CONFIG REQUIRED)
link_libraries(gRPC::grpc++)
message(STATUS "Using Protobuf ${Protobuf_VERSION}")

include_directories(${GENERATED_PROTOBUF_PATH})

# lars_reactor
set(LARS_REACTOR_INCLUDE ${LARS_PATH}/lars_reactor/include)
set(LARS_REACTOR_LIB ${LARS_PATH}/lars_reactor/build/src/libreactor.a)

# 指定二进制输出
set(THIRDPARTY ${CMAKE_BINARY_DIR}/third-party)
file(MAKE_DIRECTORY ${THIRDPARTY})

include_directories(${PROJECT_SOURCE_DIR}/include)

add_subdirectory(../third-party/ ${THIRDPARTY})
add_subdirectory(src/)
add_subdirectory(examples/)
