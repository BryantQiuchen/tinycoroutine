#client
add_executable(client_test client.cc)
target_link_libraries(client_test netco)

#server
add_executable(server_test server.cc)
target_link_libraries(server_test netco)

#mutex
add_executable(mutex_test mutex.cc)
target_link_libraries(mutex_test netco)