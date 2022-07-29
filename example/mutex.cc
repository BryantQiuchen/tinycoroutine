#include <sys/sysinfo.h>

#include <iostream>

#include "../include/mutex.h"
#include "../include/netco_api.h"
#include "../include/processor.h"
#include "../include/socket.h"

using namespace netco;
//读写锁
void mutex_test(netco::RWMutex& mu) {
  for (int i = 0; i < 10; ++i)
    if (i < 5) {
      netco::co_go([&mu, i] {
        mu.rlock();
        std::cout << i << " : start reading" << std::endl;
        netco::co_sleep(100);
        std::cout << i << " : finish reading" << std::endl;
        mu.runlock();
        mu.wlock();
        std::cout << i << " : start writing" << std::endl;
        netco::co_sleep(100);
        std::cout << i << " : finish writing" << std::endl;
        mu.wunlock();
      });
    } else {
      netco::co_go([&mu, i] {
        mu.wlock();
        std::cout << i << " : start writing" << std::endl;
        netco::co_sleep(100);
        std::cout << i << " : finish writing" << std::endl;
        mu.wunlock();
        mu.rlock();
        std::cout << i << " : start reading" << std::endl;
        netco::co_sleep(100);
        std::cout << i << " : finish reading" << std::endl;
        mu.runlock();
      });
    }
}

int main() {
  netco::RWMutex mu;
  mutex_test(mu);

  netco::sche_join();
  std::cout << "end" << std::endl;
  return 0;
}
