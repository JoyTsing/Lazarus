file(GLOB_RECURSE all_tests *.cpp)
file(GLOB_RECURSE all_src CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cpp)
list(REMOVE_ITEM all_src ${PROJECT_SOURCE_DIR}/src/dns_service.cpp)

foreach(v ${all_tests})
  string(REGEX MATCH "tests/.*" relative_path ${v})
  string(REGEX REPLACE "tests/" "" target_name ${relative_path})
  string(REGEX REPLACE ".cpp" "" target_name ${target_name})

  add_executable(${target_name} ${v} ${all_src})
  target_link_libraries(${target_name} PUBLIC ${LARS_REACTOR_LIB} doctest nanobench)
endforeach()