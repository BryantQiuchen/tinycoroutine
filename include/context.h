//@author Ryan Xu
#pragma once
#include <ucontext.h>
#include "parameter.h"
#include "utils.h"

namespace netco {
// 封装了ucontext上下文切换的一些操作

class Processor;
class Context {
 public:
  Context(size_t stackSize);
  ~Context();

  Context(const Context& otherCtx)
      : ctx_(otherCtx.ctx_), pStack_(otherCtx.pStack_) {}

  Context(Context&& otherCtx)
      : ctx_(otherCtx.ctx_), pStack_(otherCtx.pStack_) {}

  //禁止使用拷贝赋值函数
  Context& operator=(const Context& otherCtx) = delete;

  //函数指针设置当前context的上下文入口
  void makeContext(void (*func)(), Processor*, Context*);

  //直接用当前程序状态设置当前context的上下文
  void makeCurContext();

  //将当前上下文保存到oldCtx中，然后切换到当前上下文，若oldCtx为空，则直接运行
  void swapToMe(Context* pOldCtx);

  //获取当前上下文的ucontext_t指针
  inline ucontext_t* getUCtx() { return &ctx_; };

 private:
	//上下文结构
  ucontext_t ctx_;

	//栈指针
  void* pStack_;

	//协议栈大小
  size_t stackSize_;
};

}  // namespace netco