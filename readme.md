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



