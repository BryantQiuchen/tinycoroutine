# netco 协程库

## ucontext 组件

在类 System V 环境中,在头文件 `< ucontext.h >`  中定义了两个结构类型 `mcontext_t` 和 `ucontext_t` 和四个函数`getcontext(), setcontext(), makecontext(), swapcontext()` 利用它们可以在一个进程中实现用户级的线程切换。

`ucontext` 结构体：

```c++
typedef struct ucontext {
	struct ucontext *uc_link;		
    // 当前context执行结束后要执行的下一个context,如果为空,执行完当前context之后退出程序
	sigset_t         uc_sigmask;	
    // 该上下文中的阻塞信号集合（信号掩码）
	stack_t          uc_stack;
    // 当前context运行的栈信息
	mcontext_t       uc_mcontext;	
    // 保存具体的程序执行上下文,如PC值、堆栈指针以及寄存器等信息,实现依赖于底层,是平台硬件相关的
	...
} ucontext_t;
```

当前上下文运行终止时会恢复 `uc_link` 指向的上下文。

**四个函数：**

```c++
int getcontext(ucontext_t *ucp);
// 初始化 ucp 结构体，将当前的上下文保存到 ucp 中，以便后续的恢复上下文
```

```c++
int setcontext(const ucontext_t *ucp);
// 将当前程序切换到新的context,在执行正确的情况下该函数直接切换到新的执行状态,不会返回(到main)
```

`setcontext` 的上下文 ucp 通过 `getcontext` 或 `makecontext` 取得，如果调用成功则不返回。

通过 `getcontext` 取得，会继续执行这个调用；

通过 `makecontext` 取得，会调用 `makecontext` 第二参数中指向的函数。

```c++
void makecontext(ucontext_t *ucp, void (*func)(), int argc, ...);
```

调用 `makecontext` 之前需要先调用 `getcontext`，然后给该上下文指定一个栈空间 `ucp->stack`，设置后继的上下文 `ucp->uc_link`。(**不设置后继上下文，不能实现切换**)

当上下文通过 `setcontext` 或者 `swapcontext` 激活后，执行 `func` 函数，`argc` 为 `func` 的参数个数，后面是 `func` 的参数序列。当 `func` 执行返回后，继承的上下文被激活，如果继承上下文为NULL时，线程退出。

```cpp
int swapcontext(ucontext_t *oucp, ucontext_t *ucp);
// 保存当前上下文到oucp结构体中，然后激活upc上下文
```

如果执行成功，`getcontext` 返回0，`setcontext` 和 `swapcontext` 不返回；

如果执行失败，`getcontext, setcontext, swapcontext` 返回-1，并设置对应的errno.

## netco 框架
### context

context 类封装了 ucontext 上下文切换的操作，其他需要使用上下文切换的地方都使用 context 类，目的是将来想使用其他库的上下文切换方法是，只需要实现该类中的方法即可，主要实现了四个方法。

```c++
void makeContext(void (*func)(), Processor*, Context*);
//函数指针设置当前context的上下文入口
void makeCurContext();
//直接用当前程序状态设置当前context的上下文
void swapToMe(Context* pOldCtx);
//将当前上下文保存到oldCtx中，然后切换到当前上下文，若oldCtx为空，则直接运行
inline struct ucontext_t* getUCtx() { return &ctx_; };
//获取当前上下文的ucontext_t指针
```

### Coroutine

协程对象，主要实现协程的几个关键方法：`resume`，`yield`，实际真正的 `yield` 由 Processor 执行，这里的 `yield` 只是修改当前协程的状态。

### ObjPool 对象池

对象池主要用于创建 coroutine 实例上，对象池每次创建对象时，会先从**内存池**中取出相应大小的块，内存池与对象大小强相关的，有一个空闲链表，每次分配空间都从空间链表中取，如果空闲链表没有内容，首先会分配 `(分配次数+40)*对象大小` 的空间，然后分成一个个的块，挂到空闲链表上，这里空闲链表节点没有使用额外的空间：效仿的 stl 的二级配置器中的方法，将数据和 `next` 指针放在了一个 `union` 中。从内存池取出所需内存块后，会判断对象是否拥有 non-trivial （显式定义默认构造、拷贝构造、... ）构造函数，没有的话直接返回，有的话使用 $placement\ new$ 构造对象。

### Epoller

该类功能很简单，一个是监视 epoll 中是否有事件发生，一个是向 epoll 中添加、修改、删除监视的 fd。值得注意的是，该类并不存储任何协程对象实体，也不维护任何协程对象实体的生命期，使用的是 LT。

初始 `vector` 长度设置为 16。

设置 close-on-exec

### Timer

定时器主要使用的 Linux 的 `timerfd_create` 创建的时钟 fd 配合一个优先队列（小根堆）实现的，原因是要求效率而没有移除协程的需求。小根堆中存放的是时间和协程对象的 pair：`std::priority_queue<std::pair<Time, Coroutine*>`。

```c++
//获取所有已经超时的需要执行的函数
void getExpiredCoroutines(std::vector<Coroutine*>& expiredCoroutines);
//在time时刻需要恢复协程pCo
void runAt(Time time, Coroutine* pCo);
//经过time毫秒恢复协程pCo
void runAfter(Time time, Coroutine* pCo);
void wakeUp();
//给timefd重新设置时间，time是绝对时间
bool resetTimeOfTimefd(Time time);
```

首先，初始化一个 `timefd_create` 一个 timefd，然后将它放入 epoll 中，如果调用 `runAt` 或 `runAfter` 时，先把新来的任务插入到小根堆中，判断是否是最近的任务，是的话调用 `resetTimeOfTimefd` 来更新时间，如果出现超时，`epoll_wait()` 会跳出阻塞，在 Processor 的主循环中首先处理的就是超时事件，方法就是与当前时间对比并取出小根堆中的协程，直到小根堆中所有任务的时间都比当前大，另外，取出来的协程会放在一个数组中，用于在 Processor 循环中执行。

定时器还有另外一个功能，就是唤醒 `epoll_wait()`，当有新的协程加入时，实际就是通过定时器来唤醒的 processor 主循环，并执行新接受的协程。

### Processor

一个 Processor 在 netco 就对应一个线程，Processor 负责存放协程 Coroutine 实体并管理其生命周期，协程运行在 Processor 的主循环上，Processor 使用 Epoller 和 Timer 进行任务调度。

其成员如下：

```cpp
std::queue<Coroutine*> newCoroutines_[2];
// newCoroutines_为双缓冲队列，一个队列存放新来的协程，另一个给Processor主循环用于执行新来的协程，执行完后就交换队列，每加入一个新的协程就会唤醒一次Processor主循环，以立即执行新来的协程。

std::vector<Coroutine*> actCoroutines_;
// 存放EventEpoller发现的活跃事件的队列，当epoll_wait被激活时，Processor主循环会尝试从Epoller中获取活跃的协程，存放在actCoroutine_队列中，然后依次恢复执行。

std::vector<Coroutine*> timerExpiredCo_;
// 存放超时的协程队列，当epoll_wait被激活时，Processor主循环会首先尝试从Timer中获取活跃的协程，存放在timerExpiredCo队列中，然后依次恢复执行。

std::vector<Coroutine*> removedCo_;
// 存放被移除的协程列表，要移除某一个事件会先放在该列表中，一次循环结束才会真正delete
```

执行顺序：首先执行超时的协程，然后执行新接管的协程，然后执行 epoller 中被激活的协程，最后清理 `removerdCo_` 中的协程。

### Scheduler

调度器，指协程应该运行在哪个 Processor 上，netco 中的该类为全局单例，所执行的调度也相对比较简单，其可以让用户指定协程运行在某个 Processor 上，若用户没有指定，则挑选协程数量最少的 Processor 接管新的协程。

在 libgo 和 Golang 中，scheduler 还有一个 steal 的操作，可以将一个协程从一个 Processor 中偷到另一个 Processor 中，因为其 Processor 的主循环是允许阻塞的，并且协程的运行完全由库决定。而 netco 可以让用户指定某个协程一直运行在某个 Processor 上。

### netco_api

像 Golang 一样将 Scheduler 进一步地封装成了函数接口而不是一个对象，所以只需要包含 netco_api.h，即可调用 netco 函数风格的协程接口，而无需关心任何库中的对象。

### Socket

封装了 socket 族函数

### RWMutex

RWMutex 是用于协程同步的读写锁，读锁互相不互斥而与写锁互斥，写锁与其它的均互斥。原理是类中维护了一个队列，若互斥了则将当前协程放入队列中等待另一协程解锁时的唤醒。

## 运行

```shell
mkdir build && cd build
cmake ..
make
```