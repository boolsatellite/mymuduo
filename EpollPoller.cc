#include "EpollPoller.h"
#include "Logger.h"
#include "Channel.h"
#include "TimeStamp.h"
#include <errno.h>
#include <unistd.h>
#include <string.h>

const int knew = -1;   // Channel还没有被添加到poller中
const int kAdded = 1;  // Channel已经添加到poller中
const int kDelete = 2; // Channel已经从poller中删除

EPollPoller::EPollPoller(EventLoop *loop)
    : Poller(loop), epollfd_(epoll_create1(EPOLL_CLOEXEC)), events_(kInitEventListSize)
{
    if (epollfd_ < 0)
    {
        LOG_FATAL("epoll_create error : %d \n", errno);
    }
}

EPollPoller::~EPollPoller()
{
    close(epollfd_);
}

TimeStamp EPollPoller::poll(int timeoutMs, ChannelList *activeChannels)
{
    LOG_INFO("func : %s => fd total count:%ld \n",__FUNCTION__,channels_.size());
    int numEvents = ::epoll_wait(epollfd_,&*events_.begin(),static_cast<int>(events_.size()),timeoutMs);
    int saveErrno = errno;   //由于多线程访问可造成全局errno不明确，所以使用局部变量
    TimeStamp now(TimeStamp::now());

    if(numEvents > 0)
    {
        LOG_INFO("%d events happend \n",numEvents);
        fillActiveChannels(numEvents,activeChannels);
        if(numEvents == events_.size())
        {
            events_.resize(events_.size() * 2);
        }
    }
    else if(numEvents == 0)
    {
        LOG_DEBUG("%s timoout \n",__FUNCTION__);
    }
    else
    {
        if(saveErrno != EINTR)
        {
            errno = saveErrno;
            LOG_ERROR("EPollError::poll() error");
        }
    }
    return now;

}

void  EPollPoller::updateChannel(Channel *channel)
{
    const int index = channel->index();
    LOG_INFO("fd = %d events = %d index = %d \n",channel->fd(),channel->events(),channel->index());
    if(index == knew || index == kDelete)
    {
        int fd = channel->fd();
        if(index ==knew)
        {
            channels_[fd] = channel;
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD,channel);
    }
    else
    {
        int fd = channel->fd();
        if(channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL,channel);
            channel->set_index(kDelete);
        }
        else
        {
            update(EPOLL_CTL_MOD,channel);
        }
    }

}

void EPollPoller::removeChannel(Channel *channel)
{
    int fd = channel->fd();
    channels_.erase(fd);
    
    LOG_INFO("func : %s fd=%d \n",__FUNCTION__,channel->fd());

    int index = channel->index();
    if(index == kAdded)
    {
        update(EPOLL_CTL_DEL,channel);
    }
    channel->set_index(knew);
}

void EPollPoller::fillActiveChannels(int numEvents, ChannelList *activeChannels) const
{
    for(int i = 0 ; i < numEvents ; i++)
    {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel); 
        //EventLoop就拿到了它的Poller返回的所有发生时间的Channel列表了
    }
}

void EPollPoller::update(int operation, Channel *channel)
{
    struct epoll_event event;
    memset(&event,0,sizeof(event));
    event.events = operation;
    event.data.ptr = channel;
    
    LOG_INFO("func : %s fd=%d events=%d index=%d \n",__FUNCTION__,channel->fd(),channel->events(),channel->index());

    if(::epoll_ctl(epollfd_,operation,channel->fd(),&event) < 0)
    {
        if(operation == EPOLL_CTL_DEL)
        {
            LOG_ERROR("epoll_ctl del errno : %d \n",errno);
        }
        else
        {
            LOG_FATAL("epoll_ctl add/mod errno %d \n",errno);
        }
    }
}
