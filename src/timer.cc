//@Author Ryan Xu
#include "../include/timer.h"

#include <string.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include "../include/coroutine.h"
#include "../include/epoller.h"

using namespace netco;

Timer::Timer() : timeFd_(-1) {}

Timer::~Timer() {
  if (isTimeFdUseful()) {
    ::close(timeFd_);
  }
}

bool Timer::init(Epoller* pEpoller) {
  // CLOCK_MONOTONIC递增时钟,递增时钟则只受设置的时间值影响。
  timeFd_ = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  if (isTimeFdUseful()) {
    return pEpoller->addEv(nullptr, timeFd_, EPOLLIN | EPOLLPRI | EPOLLRDHUP);
  }
  return false;
}

void Timer::getExpiredCoroutines(std::vector<Coroutine*>& expiredCoroutines) {
  Time nowTime = Time::now();
  while (!timerCoHeap_.empty() && timerCoHeap_.top().first <= nowTime) {
    expiredCoroutines.push_back(timerCoHeap_.top().second);
    timerCoHeap_.pop();
  }

  if (!expiredCoroutines.empty()) {
    ssize_t cnt = TIMER_DUMMYBUF_SIZE;
    while (cnt >= TIMER_DUMMYBUF_SIZE) {
      cnt = ::read(timeFd_, dummyBuf_, TIMER_DUMMYBUF_SIZE);
    }
  }

  if (!timerCoHeap_.empty()) {
    Time time = timerCoHeap_.top().first;
    resetTimeOfTimefd(time);
  }
}

void Timer::runAt(Time time, Coroutine* pCo) {
  timerCoHeap_.push(std::move(std::pair<Time, Coroutine*>(time, pCo)));
  if (timerCoHeap_.top().first == time) {
    //新加入的任务是最紧急的任务则需要更改timefd所设置的时间
    resetTimeOfTimefd(time);
  }
}

//给timefd重新设置时间，time是绝对时间
bool Timer::resetTimeOfTimefd(Time time) {
  struct itimerspec newValue;
  struct itimerspec oldValue;
  memset(&newValue, 0, sizeof newValue);
  memset(&oldValue, 0, sizeof oldValue);
  newValue.it_value = time.timeIntervalFromNow();
  int ret = ::timerfd_settime(timeFd_, 0, &newValue, &oldValue);
  return ret < 0 ? false : true;
}
/**
 * 结构体itimerspec就是timerfd要设置的超时结构体，
 * 它的成员it_value表示定时器第一次超时时间，
 * it_interval表示之后的超时时间即每隔多长时间超时
 * struct itimerspec
  {
    struct timespec it_interval;
    struct timespec it_value;
  };
 * int timerfd_settime (int __ufd, int __flags,
                            const struct itimerspec *__utmr,
                            struct itimerspec *__otmr) __THROW;
 * 功能：启动或关闭由__ufd指定的定时器
 * @param:
 * __ufd 		: timerfd，由timerfd_create函数返回
 * __flags 	: 1代表设置的是绝对时间；为0代表相对时间
 * __utmr		: 指定新的超时时间，设定new_value.it_value非零则启动定时器，
 * 						否则关闭定时器，如果new_value.it_interval为0，
 * 						则定时器只定时一次，即初始那次，否则之后每隔设定时间超时一次
 * __otmr   : 不为null，则返回定时器这次设置之前的超时时间
*/

void Timer::runAfter(Time time, Coroutine* pCo) {
  Time runTime(Time::now().getTimeVal() + time.getTimeVal());
  runAt(runTime, pCo);
}

void Timer::wakeUp() { resetTimeOfTimefd(Time::now()); }