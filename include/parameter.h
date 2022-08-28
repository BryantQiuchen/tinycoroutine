//@author Ryan Xu
#pragma once
#include <stddef.h>

namespace tinyco {
namespace parameter {
//协程栈大小
const static size_t coroutineStackSize = 32 * 1024;

//获取活跃 epoll_event 数组的初始长度
static constexpr int epollerListFirstSize = 16;

// epoll_wait 阻塞时长
static constexpr int epollTimeOutMs = 10000;

//监听队列长度
constexpr static unsigned backLog = 4096;

//内存池没有空闲内存块时申请memPoolMallocObjCnt个对象大小的内存块
static constexpr size_t memPoolMallocObjCnt = 40;
}  // namespace parameter

}  // namespace tinyco