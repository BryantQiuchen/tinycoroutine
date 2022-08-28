//@author Ryan Xu
#include "../include/context.h"

#include <stdlib.h>

#include "../include/parameter.h"

using namespace tinyco;

Context::Context(size_t stackSize) : pStack_(nullptr), stackSize_(stackSize) {}

Context::~Context() {
  if (pStack_) {
    free(pStack_);
  }
}

void Context::makeContext(void (*func)(), Processor* pP, Context* pLink) {
  if (nullptr == pStack_) {
    pStack_ = malloc(stackSize_);
  }
	//获取当前的寄存器信息等并保存到ctx结构体中
  ::getcontext(&ctx_);
  ctx_.uc_stack.ss_sp = pStack_;
  ctx_.uc_stack.ss_size = parameter::coroutineStackSize;
  ctx_.uc_link = pLink->getUCtx();
  makecontext(&ctx_, func, 1, pP);
}

void Context::makeCurContext() { ::getcontext(&ctx_); }

void Context::swapToMe(Context* pOldCtx) {
  if (nullptr == pOldCtx) {
    //如果为空，就利用当前ctx结构体的存储信息刷新寄存器
    setcontext(&ctx_);
  } else {
    //否则，就保存当前的上下文环境到pOldCtx的ctx结构体中，
    //并将当前调用该swapToMe函数的Context类的ctx结构体保存的上下文环境切入CPU并执行
    swapcontext(pOldCtx->getUCtx(), &ctx_);
  }
}