#include <sys/sysinfo.h>

#include <iostream>

#include "../include/mutex.h"
#include "../include/tinyco_api.h"
#include "../include/processor.h"
#include "../include/socket.h"

using namespace tinyco;

// tinyco http response with one acceptor test
//只有一个 acceptor 的服务
void single_acceptor_server_test() {
  tinyco::co_go([] {
    tinyco::Socket listener;
    if (listener.isUseful()) {
      listener.setTcpNoDelay(true);
      listener.setReuseAddr(true);
      listener.setReusePort(true);
      if (listener.bind(nullptr, 8099) < 0) {
        return;
      }
      listener.listen();
    }
    while (1) {
      tinyco::Socket* conn = new tinyco::Socket(listener.accept());
      conn->setTcpNoDelay(true);
      tinyco::co_go([conn] {
        std::vector<char> buf;
        buf.resize(2048);
        while (1) {
          auto readNum = conn->read((void*)&(buf[0]), buf.size());
          std::string ok =
              "HTTP/1.0 200 OK\r\nServer: tinyco/0.1.0\r\nContent-Type: "
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
        tinyco::co_sleep(100);  //需要等一下，否则没发送完毕就关闭了
        delete conn;
      });
    }
  });
}

// tinyco http response with multi acceptor test
//每条线程一个acceptor的服务
void multi_acceptor_server_test() {
  auto tCnt = ::get_nprocs_conf();
  for (int i = 0; i < tCnt; ++i) {
    tinyco::co_go(
        [] {
          tinyco::Socket listener;
          if (listener.isUseful()) {
            listener.setTcpNoDelay(true);
            listener.setReuseAddr(true);
            listener.setReusePort(true);
            if (listener.bind(nullptr, 8099) < 0) {
              return;
            }
            listener.listen();
          }
          while (1) {
            tinyco::Socket* conn = new tinyco::Socket(listener.accept());
            conn->setTcpNoDelay(true);
            tinyco::co_go([conn] {
              std::string hello(
                  "HTTP/1.0 200 OK\r\nServer: tinyco/0.1.0\r\nContent-Length: "
                  "72\r\nContent-Type: "
                  "text/html\r\n\r\n<HTML><TITLE>hello</"
                  "TITLE>\r\n<BODY><P>hello word!\r\n</BODY></HTML>\r\n");
              // std::string hello("<HTML><TITLE>hello</TITLE>\r\n<BODY><P>hello
              // word!\r\n</BODY></HTML>\r\n");
              char buf[1024];
              if (conn->read((void*)buf, 1024) > 0) {
                conn->send(hello.c_str(), hello.size());
                tinyco::co_sleep(50);  //需要等一下，否则没发送完毕就关闭了
              }
              delete conn;
            });
          }
        },
        parameter::coroutineStackSize, i);
  }
}
int main() {
  multi_acceptor_server_test();
  tinyco::sche_join();
  std::cout << "end" << std::endl;
  return 0;
}
