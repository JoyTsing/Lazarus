configure_file(${PROJECT_SOURCE_DIR}/conf/lars_dns.conf ./conf/lars_dns.conf COPYONLY)

file(GLOB_RECURSE all_src *.cpp)
add_executable(dns_service ${all_src})

# mysql以及上一个模块reactor
target_link_libraries(dns_service PUBLIC ${LARS_REACTOR_LIB} mysqlclient)