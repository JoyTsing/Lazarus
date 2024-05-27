file(GLOB_RECURSE all_tests *.cpp)
file(GLOB_RECURSE all_src CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cpp)
configure_file(${PROJECT_SOURCE_DIR}/conf/server.conf ./conf/server.conf COPYONLY)

foreach(v ${all_tests})
  string(REGEX MATCH "examples/lars_reactor/.*" relative_path ${v})
  string(REGEX REPLACE "examples/lars_reactor/" "" target_name ${relative_path})
  string(REGEX REPLACE ".cpp" "" target_name ${target_name})

  add_executable(${target_name} ${v} ${all_src})
endforeach()