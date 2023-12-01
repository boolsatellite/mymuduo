# mymuduo
## nocopyable

<img src=".\img\AIDL Framwork.png" alt="&quot;D:\github\mymuduo\img\AIDL Framwork.png&quot;" style="zoom:150%;" />

当派生类定义了拷贝或移动构造函数，必须显式地调用基类的拷贝或移动构造函数，否则会调用基类的默认构造函数

```c++
class Base {
public:
    Base() { std::cout << "Base default constructor" << std::endl; }
    Base(const Base &b) { std::cout << "Base copy constructor" << std::endl; }
    Base(Base &&b) { std::cout << "Base move constructor" << std::endl; }
};

class Derived : public Base {
public:
    Derived() { std::cout << "Derived default constructor" << std::endl; }
    Derived(const Derived &d) { std::cout << "Derived copy constructor" << std::endl; }
    Derived(Derived &&d) { std::cout << "Derived move constructor" << std::endl; }
};

int main() {
    Derived d;
    std::cout << std::endl;
    Derived d0(d);
    std::cout << std::endl;
    Derived d1 = d;
    std::cout << std::endl;
    Derived d2{d};
    return 0;
}
输出：
Base default constructor
Derived default constructor

Base default constructor
Derived copy constructor

Base default constructor
Derived copy constructor

Base default constructor
Derived copy constructor
```

需要在子类中显示调用：

```c++
class Derived : public Base {
public:
    Derived() { std::cout << "Derived default constructor" << std::endl; }
    Derived(const Derived &d)
        : Base(d) { std::cout << "Derived copy constructor" << std::endl; }
    Derived(Derived &&d)
        : Base(std::move(d)) { std::cout << "Derived move constructor" << std::endl; }
};
输出：
Base default constructor
Derived default constructor

Base copy constructor
Derived copy constructor

Base copy constructor
Derived copy constructor

Base copy constructor
Derived copy constructor
```

当派生类未定义拷贝或移动构造函数，若基类有拷贝和移动构造函数，则派生类将获得合成的拷贝和移动构造函数，且调用的是基类的拷贝或移动构造函数

```c++
class Base {
public:
    Base() { std::cout << "Base default constructor" << std::endl; }
    Base(const Base &b) { std::cout << "Base copy constructor" << std::endl; }
    Base(Base &&b) { std::cout << "Base move constructor" << std::endl; }
};
class Derived : public Base { }
};
int main() {
    Derived d;
    Derived d1(d);
    Derived d2(std::move(d));
    return 0;
}
输出：
Base default constructor
Base copy constructor
Base move constructor
```



## Logger

```c++
#define myprintf(format, ...) fprintf (stderr, format, ##__VA_ARGS__)
```

`##__VA_ARGS__`如果可变参数被忽略或为空，## 操作将使预处理器（preprocessor）去除掉它前面的那个逗号. 如果你在宏调用时，确实提供了一些可变参数，GNU CPP 也会工作正常，它会把这些可变参数放到逗号的后面。`##__VA_ARGS__`使用



```c
time_t time(time_t * seconds);
```

回自纪元 Epoch（1970-01-01 00:00:00 UTC）起经过的时间，以秒为单位。如果 seconds 不为空，则返回值也存储在变量 seconds 中。

## InerAddress

const修饰函数返回值。
  1、函数返回const指针，表示该指针不能被改动，只能把该指针赋给const修饰的同类型指针变量。
  2、函数返回值为值传递，函数会把返回值赋给外部临时变量，用const无意义！不管是内部还是非内部数据类型。
  3、函数采用引用方式返回的场合不多，只出现在类的赋值函数中，目的是为了实现链式表达。

## Channel

listen fd，有新连接请求，\**对端发送普通数据\** 触发EPOLLIN。
带外数据，只触发EPOLLPRI。
对端正常关闭（程序里close()，shell下kill或ctr+c），触发EPOLLIN和EPOLLRDHUP，但是不触发EPOLLERR 和EPOLLHUP
对端异常断开连接（只测了拔网线），没触发任何事件

![](D:\github\mymuduo\img\Snipaste_2023-11-10_22-30-57.png)

> tie() 是如何被触发的？

## Poller

muduo在Poller.h中声明了`static Poller* newDefaultPoller(EventLoop* loop);`但未在对应的cc文件中定义，而是单独在poller/DefaultPoller.cc中定义，newDefaultPoller函数是用来返回一个EPollPoller或PollPoller的指针，而Poller属于基类，实现该函数必然要使用头文件EPollPoller，然而在基类中包含派生类的做法是不好的，故在新文件中定义

## CuurentThread

``` c
int nanosleep(const struct timespec *rqtp, struct timespec *rmtp);
```

`nanosleep()`函数会导致当前的线程将暂停执行,直到`rqtp`参数所指定的时间间隔。或者在指定时间间隔内有信号传递到当前线程，将引起当前线程调用信号捕获函数或终止该线程。



`__builtin_expect((x),0)`表示 x 的值为假的可能性更大。
编译器在编译过程中，会将可能性更大的代码紧跟着起面的代码，从而减少指令跳转带来的性能上的下降。



在多线程中，`pthread_self()`函数获得的线程号是`pthread`库对线程的编号，而不是Linux系统对线程的编号。`pthread_create()`返回的线程号，使用top命令是查不到的，top显示的是Linux的线程号。
与`getpid()`函数不同的是，Linux并没有直接给一个`gettid()`的API，而是使用syscall()直接用`SYS_gettid`的系统调用号去获取线程号。

```c
__thread int t_cachedTid = static_cast<pid_t>(syscall(SYS_gettid));
```

## EventLoop

可以使用std::ostringstream类用于处理字符串

```c++
std::string Channel::eventsToString(int fd, int ev) const {
    std::ostringstream oss;
    oss << fd << ": ";
    if (ev & EPOLLIN)         oss << "IN ";
    if (ev & EPOLLPRI)        oss << "PRI ";
    if (ev & EPOLLOUT)        oss << "OUT ";
    if (ev & EPOLLHUP)        oss << "HUP ";
    if (ev & EPOLLRDHUP)      oss << "RDHUP ";
    if (ev & EPOLLERR)        oss << "ERR ";
    return oss.str();
}
```



eventfd 是 Linux 的一个系统调用，创建一个文件描述符用于事件通知

```c
#include <sys/eventfd.h>
int eventfd(unsigned int initval, int flags);
//eventfd()调用返回一个新的fd，指向新创建的这个eventfd对象。
//eventfd对象包含一个uint64_t的计数器，由内核保存，初始化eventfd时可以由initval参数来初始化这个计数器的值。
//flags参数可以由下面几个选项按位与组合得到，决定eventfd的行为：
//EFD_CLOEXEC（linux 2.6.27及以后） 将此fd设置为 close-on-exec （调用exec时自动关闭fd）
//EFD_NONBLOCK（linux 2.6.27及以后）将fd设置为非阻塞
//EFD_SEMAPHORE（linux 2.6.30及以后）从eventfd读出类似信号量的数据，见下面关于 read 的描述
```

相关IO方法:

read成功后返回8字节整形的长度（即返回8）。读取时，如果提供的buffer小于8个字节，返回-1，则errno设置为 EINVAL 错误。

read的结果根据`eventfd`的`counter`是否为0，以及创建`eventfd`对象时flag参数是否设置了`EFD_SEMAPHORE`，有所不同。

- 如果未设置`EFD_SEMAPHORE`且counter的值非0，则read返回一个8字节整形，值是counter的值，并且将counter的值设置为0
- 如果设置了`EFD_SEMAPHORE`且`counter`的值非0，则read返回一个8字节整形，值是1，并且将`counter`的值减一
- 如果`counter`的值是0，则根据`flag`是否设置了`nonblocking`，让进程进入阻塞状态或者返回`EAGAIN`的`errno`

write方法可以将`buffer`中的8字节整形数据加到`eventfd`的`counter`上。`counter`上存储的最大值是 `unint64-1`，即`0xfffffffffffffffe`。如果相加时超限了，则根据`flag`是否设置为非阻塞，`wirte`会导致阻塞或者返回 `EAGAIN` 的 errno。

如果提供给`write`调用的`buffe`r小于8字节，或者尝试写入`0xffffffffffffffff`，write会返回 EINVAL 错误。

eventfd支持`poll`、`select`、`epoll`等类似操作。

- 当`counter`的值大于0时，`eventfd`是可读的
- 当`counter`小于`0xffffffffffffffff`，即至少可以写入一个1而不阻塞时，`eventfd`是可写的
- 当counter溢出时，`selec`t认为`eventfd`即是可写的又是可读的，`poll`则会返回 `POLLERR` 错误。如上所述，`write`永远不会导致`counter`溢出。但是，如果` KAIO `子系统执行了` 2^64` 个` eventfd`信号发布”，则可能会发生溢出（理论上可能，但实际上不太可能）。 如果发生溢出，则 `read` 将返回该最大 `uint64_t` 值（即 `0xffffffffffffffff`）。



### Thread

当类成员需要线程对象时，不可以直接使用线程对象，而是使用指向线程对象的指针，因为`std::thread`一旦创建线程就开启了



>  对于成员变量numCreated_ 只看到自增却看不到自减?

陈硕大神使用了自实现的互斥锁，条件变量，以及计数门杉

### EventLoopThread

**one loop pre thread 代码体现**

```c++
void Thread::start() {
    sem_t sem;
    sem_init(&sem , false , 0);
    thread_ = std::make_shared<std::thread>([&](){
        tid_ = CurrentThread::tid();    //获取新开启线程的tid
        sem_post(&sem);
        func_();
    });
    //这里必须等待上面新创建的线程的tid值(用于标识线程成功创建)，采用信号量解决
    sem_wait(&sem);
    sem_destroy(&sem);
}

EventLoop *EventLoopThread::startLoop() {
    assert(!thread_.started());
    thread_.start();        //开启新线程执行 EventLoopThread::threadFunc
    EventLoop* loop = nullptr;
    {
         std::unique_lock<std::mutex> lock(mutex_);
         while(loop_ != nullptr) {
             cond_.wait(lock);      //解锁等待notify
         }
         loop = loop_;
    }
    return loop;
}

void EventLoopThread::threadFunc() {    //在单独的新线程内运行
    EventLoop loop;         //在新线程内创建一个EventLoop， one loop pre thread
    if(callback_) {
        callback_(&loop);   //调用线程初始化回调函数
    }
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }
    loop.loop();            //开启Poller

    //当可以执行到这里，说明poller::poll() 结束了，事件循环结束了
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}
```



### EventLoopThreadPool

```c++
void EventLoopThreadPool::start(const ThreadInitCallback& cb)
{
  assert(!started_);
  baseLoop_->assertInLoopThread();

  started_ = true;

  for (int i = 0; i < numThreads_; ++i)
  {
    char buf[name_.size() + 32];				//使用了变量开辟数组
    snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
    EventLoopThread* t = new EventLoopThread(cb, buf);
    threads_.push_back(std::unique_ptr<EventLoopThread>(t));
    loops_.push_back(t->startLoop());
  }
  if (numThreads_ == 0 && cb)
  {
    cb(baseLoop_);
  }
}
```

C/C++语法规范中，不能使用变量定义数组维数，因为数组维数的确定，需要在编译阶段完成。要想在程序实行阶段确定数组维数，应该使用new/malloc去动态分配；但是试验了一下，由于g++对C99的支持，使得使用变量定义数组维数也是可行的(在Linux系统下)，但是如此方法，在VC下会报错，即这并不是符合C++语法规范的写法，在很大程度上会限制代码的平台兼容性，应该避免这种写法

> ```c++
> std::vector<EventLoop *> EventLoopThreadPool::getAllLoops()
> ```
>
> 调用此函数，有没有可能导致 EventLoop的析构函数被重复调用

### Socket

TCP连接的TIME_WAIT状态，服务器程序可以通过设置`socket`选项`SO_REUSEADDR`来强制使用被处于` TIME_WAIT`状态的连接占用的`socket`地址

```c++
void Socket::setReuseAddr(bool on) const {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_ , SOL_SOCKET , SO_REUSEADDR , &optval , sizeof optval);
}
```

端口复用允许在一个应用程序可以把 n 个套接字绑在一个端口上而不出错。同时，这 n 个套接字发送信息都正常，没有问题。详细参考下方链接

```c++
void Socket::setReusePort(bool on) {
  int optval = on ? 1 : 0;
  int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT,
                         &optval, static_cast<socklen_t>(sizeof optval));
  if (ret < 0 && on) {
    LOG_SYSERR << "SO_REUSEPORT failed.";
  }
}
```

当一个 TCP 连接建立之后，开启 TCP keepalive 的一端会启动一个计时器，当这个计时器数值到达 0 之后（也就是经过 tcp_keepalive_time 时间后，当然每次传输数据都将重置计时器数值），会发送一个保活探测报文。探测报文不包含任何数据，或者包含一个无意义的字节

```c++
void Socket::setKeepAlive(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE,
               &optval, static_cast<socklen_t>(sizeof optval));
}
```



## Buffer

```c
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
```

对于扩容函数：

```c++

  void makeSpace(size_t len) {
    if (writableBytes() + prependableBytes() < len + kCheapPrepend) {
      // FIXME: move readable data
      buffer_.resize(writerIndex_+len);
    }
    else {
      // move readable data to the front, make space inside buffer
      assert(kCheapPrepend < readerIndex_);
      size_t readable = readableBytes();
      std::copy(begin()+readerIndex_,
                begin()+writerIndex_,
                begin()+kCheapPrepend);
      readerIndex_ = kCheapPrepend;
      writerIndex_ = readerIndex_ + readable;
      assert(readable == readableBytes());
    }
  }
```

`prependableBytes()` 返回 `readerIndex_` 的值，此条件判断可以是：`buffer_.size() - writerIndex_ + readerIndex_ - kCheapPrepend`，即可写部分加上读缓冲中不被使用的部分，若这部分空间小于len就扩容，直接在写空间上扩容而不考虑利用读缓冲中废弃的部分，若这部分空间大于len，说明可以存放len字节，就将数据前移读缓冲中被废弃的字节数



**readv writev**

`struct iovec`定义了一个向量元素。通常，这个结构用作一个多元素的数组。对于每一个传输的元素，指针成员iov_base指向一个缓冲区，这个缓冲区是存放的是readv所接收的数据或是`writev`将要发送的数据。成员`iov_len`在各种情况下分别确定了接收的最大长度以及实际写入的长度。且`iovec`结构是用于`scatter/gather IO`的。`readv和writev`函数用于在一次函数调用中读、写多个非连续缓冲区。有时也将这两个函数称为散布读（scatter read）和聚集写（gather write）。

```c
#include <sys/uio.h>
struct iovec {
    ptr_t iov_base; /* Starting address */
    size_t iov_len; /* Length in bytes */
};

ssize_t readv(int fd, const struct iovec *iov, int iovcnt);
ssize_t writev(int fd, const struct iovec *iov, int iovcnt);
```

这些函数的返回值是`readv`所读取的字节数或是`writev`所写入的字节数。如果有错误发生，就会返回-1



### TcpConnection

EPIPE / ECONNRESET

"write error: EPIPE" 是一种常见的 UNIX/Linux 错误，通常在管道或套接字上写入数据时出现。它表示在写入数据时，读取数据的进程已经终止，因此写入操作失败。在代码中，可以通过捕获 `SIGPIPE` 信号并对其进行处理，以避免出现这种错误。















https://www.zoucz.com/blog/2022/06/14/2c0ff480-ebd4-11ec-bbfb-55427a78e3a0/

https://dlonng.com/posts/semaphore

https://cloud.tencent.com/developer/article/1963171   端口复用

https://juejin.cn/post/7142654287493988359			tcp保活
