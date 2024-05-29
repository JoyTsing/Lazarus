cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
  message("Setting default build type to Debug")
endif()

project(lars_reactor VERSION 0.10 LANGUAGES CXX)

include_directories(${PROJECT_SOURCE_DIR}/include)

# 指定二进制输出
set(THIRDPARTY ${CMAKE_BINARY_DIR}/third-party)
file(MAKE_DIRECTORY ${THIRDPARTY})

add_subdirectory(../third-party/ ${THIRDPARTY})
add_subdirectory(src/)
add_subdirectory(examples/)
add_subdirectory(tests/)
