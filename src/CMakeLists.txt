add_library(tinyco SHARED
    context.cc
    coroutine.cc
    epoller.cc
    timer.cc
    mstime.cc
    mutex.cc
    processor.cc
    processor_selector.cc
    socket.cc
    scheduler.cc
    netco_api.cc
    tcp_server.cc
    tcp_client.cc
    )