#include "EpollPoller.h"
#include "Logger.h"
#include <errno.h>
#include <unistd.h>

const int knew = -1;   //Channel还没有被添加到poller中
const int kAdded = 1;  ///Channel已经添加到poller中
const int kDelete = 2; //Channel已经从poller中删除


EPollPoller::EPollPoller(EventLoop* loop)
    :Poller(loop)
    ,epollfd_(epoll_create1(EPOLL_CLOEXEC))
    ,events_(kInitEventListSize)
{
    if(epollfd_ < 0)
    {
        LOG_FATAL("epoll_create error : %d \n",errno);
    }
}

EPollPoller::~EPollPoller()
{
    close(epollfd_);
}



