#include <sys/sysinfo.h>

#include <iostream>

#include "../include/mutex.h"
#include "../include/tinyco_api.h"
#include "../include/processor.h"
#include "../include/socket.h"

using namespace tinyco;
//读写锁
void mutex_test(tinyco::RWMutex& mu) {
  for (int i = 0; i < 10; ++i)
    if (i < 5) {
      tinyco::co_go([&mu, i] {
        mu.rlock();
        std::cout << i << " : start reading" << std::endl;
        tinyco::co_sleep(100);
        std::cout << i << " : finish reading" << std::endl;
        mu.runlock();
        mu.wlock();
        std::cout << i << " : start writing" << std::endl;
        tinyco::co_sleep(100);
        std::cout << i << " : finish writing" << std::endl;
        mu.wunlock();
      });
    } else {
      tinyco::co_go([&mu, i] {
        mu.wlock();
        std::cout << i << " : start writing" << std::endl;
        tinyco::co_sleep(100);
        std::cout << i << " : finish writing" << std::endl;
        mu.wunlock();
        mu.rlock();
        std::cout << i << " : start reading" << std::endl;
        tinyco::co_sleep(100);
        std::cout << i << " : finish reading" << std::endl;
        mu.runlock();
      });
    }
}

int main() {
  tinyco::RWMutex mu;
  mutex_test(mu);

  tinyco::sche_join();
  std::cout << "end" << std::endl;
  return 0;
}
