//@author Liu Yukang
#include "../include/tinyco_api.h"

void tinyco::co_go(std::function<void()>&& func, size_t stackSize, int tid) {
  if (tid < 0) {
    tinyco::Scheduler::getScheduler()->createNewCo(std::move(func), stackSize);
  } else {
    tid %= tinyco::Scheduler::getScheduler()->getProCnt();
    tinyco::Scheduler::getScheduler()->getProcessor(tid)->goNewCo(
        std::move(func), stackSize);
  }
}

void tinyco::co_go(std::function<void()>& func, size_t stackSize, int tid) {
  if (tid < 0) {
    tinyco::Scheduler::getScheduler()->createNewCo(func, stackSize);
  } else {
    tid %= tinyco::Scheduler::getScheduler()->getProCnt();
    tinyco::Scheduler::getScheduler()->getProcessor(tid)->goNewCo(func,
                                                                 stackSize);
  }
}

void tinyco::co_sleep(Time time) {
  tinyco::Scheduler::getScheduler()->getProcessor(threadIdx)->wait(time);
}

void tinyco::sche_join() { tinyco::Scheduler::getScheduler()->join(); }