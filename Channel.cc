#include "Channel.h"
#include "Logger.h"
#include "EventLoop.h"

#include<sys/epoll.h>
const int Channel::KNoneEvent = 0;
const int Channel::KReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::KWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop* loop , int fd)
    :loop_(loop),
    fd_(fd),
    events_(0),
    revents_(0),
    index_(-1),      //注意将index_初始化为 -1
    tied_(false)
{}

Channel::~Channel() {}

//tie在TcpConnection新连接创建时被调用
void Channel::tie(const std::shared_ptr<void>& obj)
{
    tie_ = obj;
    tied_ = true;
}

void Channel::update()
{
    //通过channel中所属的EventLoop调用poller对应的方法，注册fd的events
    //add code
    loop_->updateChannel(this);
}

void Channel::remove()
{
    //在Channel所属的EventLoop中将当前的Channel删除掉
    //add code
    loop_->removeChannel(this);
}

void Channel::handleEvent(TimeStamp receiveTime)
{
    std::shared_ptr<void> guard;
    if(tied_)
    {
        guard = tie_.lock();
        if(guard)
        {
            handleEventWithGuard(receiveTime);
        }
    }
    else
    {
        handleEventWithGuard(receiveTime);
    }
}

void Channel::handleEventWithGuard(TimeStamp receiveTime)
{
    LOG_INFO("channel handleEvent revents : %d \n",revents_);

    if((revents_ & EPOLLHUP) && (revents_ & EPOLLIN))
    {
        if(closeCallback)
        {
            closeCallback();
        }
    }

    if(revents_ & EPOLLERR)
    {
        if(errorCallback)
        {
            errorCallback();
        }
    }

    if(revents_ & EPOLLIN)
    {
        if(readCallback)
        {
            readCallback(receiveTime);
        }
    }

    if(revents_ & EPOLLOUT)
    {
        if(writeCallback)
        {
            writeCallback();
        }
    }
}

