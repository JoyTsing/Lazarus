file(GLOB_RECURSE all_tests *.cpp)
file(GLOB_RECURSE all_src CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cpp)

foreach(v ${all_tests})
  string(REGEX MATCH "tests/.*" relative_path ${v})
  string(REGEX REPLACE "tests/" "" target_name ${relative_path})
  string(REGEX REPLACE ".cpp" "" target_name ${target_name})

  add_executable(${target_name} ${v} ${all_src})
  target_link_libraries(${target_name} PUBLIC doctest nanobench
    Backward::Backward Backward::Object ${Boost_LIBRARY_DIRS})
endforeach()