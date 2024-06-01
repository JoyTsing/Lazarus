cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
  message("Setting default build type to Debug")
endif()

project(lazarus VERSION 0.1 LANGUAGES CXX)

add_subdirectory(lars_reactor/)
add_subdirectory(lars_reporter/)
add_subdirectory(lars_dns/)
add_subdirectory(lars_loadbalance/)