# api
set(LAZARUS_LIB ${CMAKE_BINARY_DIR}/src/liblazarusclient.a)

include_directories(${LARS_REACTOR_INCLUDE})

# taget
add_custom_target(
  EXAMPLE_DEP
  DEPENDS
  ${GENERATED_PROTOBUF_FILES}
  ${LAZARUS_LIB}
  VERBATIM
)

file(GLOB_RECURSE all_src ${PROJECT_SOURCE_DIR}/examples/*.cpp)

foreach(v ${all_src})
  string(REGEX MATCH "examples/.*" relative_path ${v})
  string(REGEX REPLACE "examples/" "" target_name ${relative_path})
  string(REGEX REPLACE ".cpp" "" target_name ${target_name})
  add_executable(${target_name} ${v} ${GENERATED_PROTOBUF_FILES})
  add_dependencies(${target_name} EXAMPLE_DEP)
  target_link_libraries(${target_name} PUBLIC ${LAZARUS_LIB} protobuf::libprotobuf Backward::Backward Backward::Object)
endforeach()