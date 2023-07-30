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

**std::copy**

```c++
template< class InputIt, class OutputIt >
OutputIt copy( InputIt first, InputIt last, OutputIt d_first );
```

作用：复制 `[first, last)` 所定义的范围中的元素到始于 `d_first` 的另一范围。

返回：指向目标范围中最后复制元素的下个元素的输出迭代器。

**设置地址重用**

```c++
int optval = on ? 1 : 0;
::setsockopt(sockfd_ , SOL_SOCKET, SO_REUSEPORT,
                &optval , static_cast<socklen_t>(sizeof optval));
```

一般来说，一个端口释放后会等待两分钟之后才能再被使用，SO_REUSEADDR是让端口释放后立即就可以被再次使用

`SO_REUSEADDR`用于对TCP套接字处于TIME_WAIT状态下的socket，才可以重复绑定使用

> TCP，先调用close()的一方会进入TIME_WAIT状态

SO_REUSEADDR 提供如下四个功能：

- 允许启动一个监听服务器并捆绑其众所周知端口，即使以前建立的将此端口用做他们的本地端口的连接仍存在。这通常是重启监听服务器时出现，若不设置此选项，则bind时将出错
- 允许在同一端口上启动同一服务器的多个实例，只要每个实例捆绑一个不同的本地IP地址即可。对于TCP，我们根本不可能启动捆绑相同IP地址和相同端口号的多个服务器。
- 允许单个进程捆绑同一端口到多个套接口上，只要每个捆绑指定不同的本地IP地址即可。这一般不用于TCP服务器。
- SO_REUSEADDR允许完全重复的捆绑：
  当一个IP地址和端口绑定到某个套接口上时，还允许此IP地址和端口捆绑到另一个套接口上。一般来说，这个特性仅在支持多播的系统上才有，而且只对UDP套接口而言（TCP不支持多播）。

**端口重用**

```c++
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_ , SOL_SOCKET, SO_REUSEPORT,
                &optval , static_cast<socklen_t>(sizeof optval));
```

> 那么什么是端口复用呢，如何理解呢，可以解释成如下： 
> 在A机上进行客户端网络编程，假如它所使用的本地端口号是1234，如果没有开启端口复用的话，它用本地端口1234去连接B机再用本地端口连接C机时就不可以，若开启端口复用的话在用本地端口1234访问B机的情况下还可以用本地端口1234访问C机。若是服务器程序中监听的端口，即使开启了复用，也不可以用该端口望外发起连接了。

有如下语义：

此选项允许完全重复捆绑，但仅在想捆绑相同IP地址和端口的套接口都指定了此套接口选项才行。

如果被捆绑的IP地址是一个多播地址，则SO_REUSEADDR和SO_REUSEPORT等效。

使用这两个套接口选项的建议：

- 在所有TCP服务器中，在调用bind之前设置SO_REUSEADDR套接口选项；
- 当编写一个同一时刻在同一主机上可运行多次的多播应用程序时，设置SO_REUSEADDR选项，并将本组的多播地址作为本地IP地址捆绑

**创建非阻塞socket**

```c++
int sockfd = socket(AF_INET,SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC , 0);
```

在linux内核2.6.27以后type参数有个第二种功能，除了指定套接字类型以外它还提供了```SOCK_NONBLOCK  SOCK_CLOEXEC```的比特位的或运算，以修改socket()的行为，```SOCK_NONBLOCK  ```设置该套接字为非阻塞文件```SOCK_CLOEXEC```设置在子进程中不继承该文件描述符

**粘包问题**

TCP粘包、拆包属于网络底层问题，在数据链路层、网络层、传输层都有可能出现。日常的网络应用开发大多数在传输层出现，而UDP是由消息保护边界的，不会发生粘包、拆包问题，只发生在TCP协议中。假设客户端向服务端发送了两个连续的数据包Packet1、Packet2；

在这个过程中可能会出现3种情况：

- 正常：两个数据包逐一分开发送
- 粘包：两个包一同发送，
- 拆包：Server接收到不完整的或多出一部分的数据包

![](.\Snipaste_2023-07-30_12-52-37.png)

TCP本身是面向流的，作为网络服务器，如何从这源源不断涌来的数据流中拆分出或者合并出有意义的信息呢？通常会有以下一些常用的方法：

1. 发送端给每个数据包添加包首部，首部中应该至少包含数据包的长度，这样接收端在接收到数据后，通过读取包首部的长度字段，便知道每一个数据包的实际长度了。
2. 发送端将每个数据包封装为固定长度（不够的可以通过补0填充），这样接收端每次从接收缓冲区中读取固定长度的数据就自然而然的把每个数据包拆分开来。
3. 可以在数据包之间设置边界，如添加特殊符号，这样，接收端通过这个边界就可以将不同的数据包拆分开。

**const类成员函数注意点**

```c++
   class A{
	char *begin()
    {
        return &*buffer_.begin();
    }

    const char* begin() const   //const 成员函数只能返回指向成员的const指针或引用。
    {
        return &(*buffer_.begin());
    }
	};
```

begin存在const重载，但是对于const重载的函数返回值必须是const类型，这是为了防止内部内部成员变量被修改，当返回内部成员变量的指针或引用时必须返回const类型，其他情况则任意(按值传递)



