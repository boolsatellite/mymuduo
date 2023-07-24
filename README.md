## muduo库核心代码实现零散知识

**nocopyable**

muduo库中为了阻止某些类的拷贝构造和赋值采用了私有继承nocopyable类，nocopyable将拷贝构造和赋值运算符delete掉了，这样有效的阻止了派生类的复制，优于将拷贝构造delete或设为私有方法，这种方法可以清晰的看出一个类是否允许被复制

**__thread 关键字**

```__thread```是GCC内置的线程局部存储设施，存取效率可以和全局变量相比。 ```__thread```变量每一个线程有一份独立实体，各个线程的值互不干扰。可以用来修饰那些带有全局性且值可能变，但是又不值得用全局变量保护的变量。

**__builtin_expect 说明**

这个指令是gcc引入的，作用是**允许程序员将最有可能执行的分支告诉编译器**。这个指令的写法为：`__builtin_expect(EXP, N)`。
意思是：EXP==N的概率很大。

通过这种方式，编译器在编译过程中，会将可能性更大的代码紧跟着起面的代码，从而减少指令跳转带来的性能上的下降。

**获取真实的tid**

```pthread_t pthread_self(void)```返回进程内唯一的线程id并非操作系统层面的线程id

获取真实的```tid```应该使用系统调用

```c++
int tid = static_cast<pid_t>(syscall(SYS_gettid));
```

**eventfd**

描述：

>  自内核 2.6.22 起，Linux 通过 `eventfd()` 系统调用额外提供了一种非标准的同步机制。这个系统调用创建了一个 `eventfd` 对象，该对象拥有一个相关的由内核维护的 8 字节无符号整数。通知机制就建立在这个无符号整数的数值变化上。
>
> 这个系统调用返回一个指向该 `eventfd` 的文件描述符。用户可以对这个文件描述符使用 `read` 或 `write` 系统调用，来操作由内核维护的数值。
>
> 此外，`eventfd` 可以和 `epoll` 等多路复用的系统调用一同使用：我们可以使用多路复用的系统调用测试对象值是否为非零，如果是非零的话就表示文件描述符可读。
>
> 在 linux 2.6.22 之后，`eventfd` 可用。在 linux 2.6.27 之后，`eventfd2` 可用。他们二者的区别是，`eventfd` 系统调用没有 `flags` 参数。而 glibc 从2.9开始，提供的 `eventfd` 底层则会调用 `eventfd2` 来进行实现（如果 `eventfd2` 被内核支持的话）。
>
> 因此如果内核版本不支持，您务必要将 `flags` 设置为0，除此之外这两个系统调用没有差别。

使用：

> ```c
> # include<sys/eventfd.h>
> int eventfd(unsigned int initval, int flags);
> // initval：创建eventfd时它所对应的64位计数器的初始值；
> // flags：eventfd文件描述符的标志，可由三种选项组成：EFD_CLOEXEC、EFD_NONBLOCK和EFD_SEMAPHORE
>           EFD_CLOEXEC表示返回的eventfd文件描述符在fork后exec其他程序时会自动关闭这个文件描述符；
>           EFD_NONBLOCK设置返回的eventfd非阻塞；
>           EFD_SEMAPHORE表示将eventfd作为一个信号量来使用
> ```
>
> * 读eventfd
>
> 使用read函数，需要注意如下几点：
>
> （1）read函数会从eventfd对应的64位计数器中读取一个8字节的整型变量
>
> （2）read函数设置的接收buf的大小不能低于8个字节，否则read函数会出错，errno为EINVAL
>
> （3）read函数返回的值是按小端字节序的
>
> （4）如果eventfd设置了EFD_SEMAPHORE，那么每次read就会返回1，并且让eventfd对应的计数器减1；如果eventfd没有设置EFD_SEMAPHORE，那么每次read就会直接返回计数器中的数值，read之后计数器就会置0。不管是哪一种，当计数器为0时，如果继续read，那么read就会阻塞（如果eventfd没有设置EFD_NONBLOCK）或者返回EAGAIN错误（如果eventfd设置了EFD_NONBLOCK）。
>
> * 写eventfd
>
> 使用write函数：
>
> （1）在没有设置EFD_SEMAPHORE的情况下，write函数会将发送buf中的数据写入到eventfd对应的计数器中，最大只能写入0xffffffffffffffff，否则返回EINVAL错误；
>
> （2）在设置了EFD_SEMAPHORE的情况下，write函数相当于是向计数器中进行“添加”，比如说计数器中的值原本是2，如果write了一个3，那么计数器中的值就变成了5。如果某一次write后，计数器中的值超过了0xfffffffffffffffe（64位最大值-1），那么write就会阻塞直到另一个进程/线程从eventfd中进行了read（如果write没有设置EFD_NONBLOCK），或者返回EAGAIN错误（如果write设置了EFD_NONBLOCK）。



