#include "Poller.h"
#include "EpollPoller.h"
#include<stdlib.h>

/*
newdefaultPoller函数在Poller.h中声明但是并不在对应的 cc 文件中定义
这是因为，从继承角度看，Poller是基类，但是其对于的epoll和poll具体实现
是派生类，所有Poller.h中不应该包含其派生类的头文件
*/


Poller* Poller::newdefautPoller(EventLoop * loop)
{
    if(::getenv("MUDO_USE_POLL"))
    {
        // return new PollPoller(loop);
        return nullptr;  //生成Poller实例
    }
    else
    {
        return new EPollPoller(loop);
    }
}