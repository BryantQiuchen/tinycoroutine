#include <sys/sysinfo.h>

#include <iostream>

#include "../include/mutex.h"
#include "../include/netco_api.h"
#include "../include/processor.h"
#include "../include/socket.h"

using namespace netco;

// netco http response with one acceptor test
//只有一个 acceptor 的服务
void single_acceptor_server_test() {
  netco::co_go([] {
    netco::Socket listener;
    if (listener.isUseful()) {
      listener.setTcpNoDelay(true);
      listener.setReuseAddr(true);
      listener.setReusePort(true);
      if (listener.bind(8099) < 0) {
        return;
      }
      listener.listen();
    }
    while (1) {
      netco::Socket* conn = new netco::Socket(listener.accept());
      conn->setTcpNoDelay(true);
      netco::co_go([conn] {
        std::vector<char> buf;
        buf.resize(2048);
        while (1) {
          auto readNum = conn->read((void*)&(buf[0]), buf.size());
          std::string ok =
              "HTTP/1.0 200 OK\r\nServer: netco/0.1.0\r\nContent-Type: "
              "text/html\r\n\r\n";
          if (readNum < 0) {
            break;
          }
          conn->send(ok.c_str(), ok.size());
          conn->send((void*)&(buf[0]), readNum);
          if (readNum < (int)buf.size()) {
            break;
          }
        }
        netco::co_sleep(100);  //需要等一下，否则没发送完毕就关闭了
        delete conn;
      });
    }
  });
}

int main() {
  single_acceptor_server_test();
  netco::sche_join();
  std::cout << "end" << std::endl;
  return 0;
}
