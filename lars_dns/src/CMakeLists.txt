configure_file(${PROJECT_SOURCE_DIR}/conf/lars_dns.conf ./conf/lars_dns.conf COPYONLY)

add_executable(dns_service dns_service.cpp)

# mysql以及上一个模块reactor
target_link_libraries(dns_service PUBLIC ${LARS_REACTOR_LIB} mysqlclient)