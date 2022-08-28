//@author Ryan Xu
#pragma once
#include <vector>

#include "utils.h"
//监视epoll中是否有事件发生 + 向epoll中更改fd

struct epoll_event;

namespace tinyco {
class Coroutine;

class Epoller {
 public:
  Epoller();
  ~Epoller();

  DISALLOW_COPY_MOVE_AND_ASSIGN(Epoller);

  //使用EventEpoller必须调用调用该函数进行初始化，失败返回false
  bool init();

  //修改Epoller中的事件
  bool modifyEv(Coroutine* pCo, int fd, int interesEv);

  //向Epoller中添加事件
  bool addEv(Coroutine* pCo, int fd, int interesEv);

  //从Epoller中移除事件
  bool removeEv(Coroutine* pCo, int fd, int interesEv);

  //获取被激活的事件服务，返回errno
  int getActEvServ(int timeOutMs, std::vector<Coroutine*>& activeEvServs);

 private:
  inline bool isEpollFdUseful() { return epollFd_ < 0 ? false : true; };

  int epollFd_;

  //存放epoll活跃事件
  std::vector<struct epoll_event> activeEpollEvents_;
};

}  // namespace tinyco
