#client
add_executable(client_test client.cc)
target_link_libraries(client_test netco)

#single acceptor server
add_executable(single_server_test single_acceptor.cc)
target_link_libraries(single_server_test netco)

#multi acceptor server
add_executable(multi_acceptor_test multi_acceptor.cc)
target_link_libraries(multi_acceptor_test netco)

#mutex
add_executable(mutex_test mutex.cc)
target_link_libraries(mutex_test netco)