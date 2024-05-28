cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
  message("Setting default build type to Debug")
endif()

project(lars_dns VERSION 0.1 LANGUAGES CXX)

# lars_reactor
get_filename_component(LARS_REACTOR_PATH ${PROJECT_SOURCE_DIR}/../lars_reactor ABSOLUTE)
set(LARS_REACTOR_INCLUDE ${LARS_REACTOR_PATH}/include)
set(LARS_REACTOR_LIB ${LARS_REACTOR_PATH}/build/src/libreactor.a)

include_directories(${LARS_REACTOR_INCLUDE})
include_directories(${PROJECT_SOURCE_DIR}/include)

add_subdirectory(third-party/)
add_subdirectory(examples/)
add_subdirectory(tests/)
