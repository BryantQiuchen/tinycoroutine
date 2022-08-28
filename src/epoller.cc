//@author Ryan Xu

#include "../include/epoller.h"

#include <errno.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "../include/coroutine.h"
#include "../include/parameter.h"

using namespace tinyco;

Epoller::Epoller()
    : epollFd_(-1), activeEpollEvents_(parameter::epollerListFirstSize) {}

Epoller::~Epoller() {
  if (isEpollFdUseful()) {
    ::close(epollFd_);
  }
};

bool Epoller::init() {
  epollFd_ = ::epoll_create1(EPOLL_CLOEXEC);
  /*
  设置为epoll_create1(0)和 epoll_create 一致
  设置为EPOLL_CLOEXEC，那么由当前进程 fork 出来的任何子进程，子进程不能使用父进程中的fd，父进程的fd可以正常使用
  */
  return isEpollFdUseful();
}

bool Epoller::modifyEv(Coroutine* pCo, int fd, int interesEv) {
  if (!isEpollFdUseful()) {
    return false;
  }
  struct epoll_event event;
  memset(&event, 0, sizeof(event));
  event.events = interesEv;
  event.data.ptr = pCo;
  if (::epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &event) < 0) {
    return false;
  }
  return true;
}

bool Epoller::addEv(Coroutine* pCo, int fd, int interesEv) {
  if (!isEpollFdUseful()) {
    return false;
  }
  struct epoll_event event;
  memset(&event, 0, sizeof(event));
  event.events = interesEv;
  event.data.ptr = pCo;
  if (::epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &event) < 0) {
    return false;
  }
  return true;
}

bool Epoller::removeEv(Coroutine* pCo, int fd, int interesEv) {
  if (!isEpollFdUseful()) {
    return false;
  }
  struct epoll_event event;
  memset(&event, 0, sizeof(event));
  event.events = interesEv;
  event.data.ptr = pCo;
  if (::epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, &event) < 0) {
    return false;
  }
  return true;
}

int Epoller::getActEvServ(int timeOutMs,
                          std::vector<Coroutine*>& activeEvServs) {
  if (!isEpollFdUseful()) {
    return -1;
  }
  int actEvNum =
      ::epoll_wait(epollFd_, &*activeEpollEvents_.begin(),
                   static_cast<int>(activeEpollEvents_.size()), timeOutMs);
  int savedErrno = errno;
  if (actEvNum > 0) {
    if (actEvNum > static_cast<int>(activeEpollEvents_.size())) {
      return savedErrno;
    }
    for (int i = 0; i < actEvNum; ++i) {
      //设置事件类型，放进活跃事件列表中
      Coroutine* pCo = static_cast<Coroutine*>(activeEpollEvents_[i].data.ptr);
      activeEvServs.push_back(pCo);
    }
    if (actEvNum == static_cast<int>(activeEpollEvents_.size())) {
      //若从epoll中获取事件的数组满了，说明数组大小不够，扩充一倍
      activeEpollEvents_.resize(activeEpollEvents_.size() * 2);
    }
  }
  return savedErrno;
}