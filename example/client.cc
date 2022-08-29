#include <sys/sysinfo.h>

#include <iostream>

#include "../include/mutex.h"
#include "../include/tinyco_api.h"
#include "../include/processor.h"
#include "../include/socket.h"

using namespace tinyco;

//作为客户端的测试，可配合上述server测试
void client_test() {
  tinyco::co_go([] {
    char buf[1024];
    while (1) {
      tinyco::co_sleep(2000);
      tinyco::Socket s;
      s.connect("127.0.0.1", 8099);
      s.send("ping", 4);
      s.read(buf, 1024);
      std::cout << std::string(buf) << std::endl;
    }
  });
}

int main() {
  client_test();
  tinyco::sche_join();
  std::cout << "end" << std::endl;
  return 0;
}
