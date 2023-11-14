//
// Created by satellite on 11/11/2023.
//

#include <cassert>
#include <cstring>
#include "EPollPoller.h"
#include "Logger.h"
#include "unistd.h"
#include "Channel.h"

//channel的状态
const int kNew = -1;    //未添加到poller中
const int kAdded = 1;   //已添加到poller中
const int kDeleted = 2;  //曾经添加到poller中但已经被删除


EPollPoller::EPollPoller(EventLoop *loop)
    : Poller(loop)                      //基类初始化
    , epollfd_(epoll_create(1))
    , events_(kInitEventListSize)       //explicit vector( size_type count,const T& value = T(),const Allocator& alloc = Allocator() );
{
    if(epollfd_ < 0) {
        LOG_FATAL("%s %s %d : error epoll_create ",debugline);
    }
}

EPollPoller::~EPollPoller() {
    close(epollfd_);
}

void EPollPoller::updateChannel(Channel *channel) {
    const int index = channel->index();     //获取channel在当前poller中的状态
    LOG_INFO("%s %s %d : fd= %d events = %d index = %d",debugline , channel->fd() , channel->events() , channel->index());
    const int fd = channel->fd();
    if(index == kNew || index == kDeleted) {
        if(index == kNew) {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;            //加入channelMap
        } else {                                //当index为delete时，并没有epollctl_del
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD , channel);
    } else {                                    //index = kAdded
        if(channel->isNoneEvent()) {
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
            assert(index == kAdded);
            update(EPOLL_CTL_DEL , channel);
            channel->set_index(kDeleted);
        } else {
            update(EPOLL_CTL_MOD , channel);
        }
    }
}

void EPollPoller::removeChannel(Channel *channel) {
    int fd = channel->fd();
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->isNoneEvent());
    int index = channel->index();
    if(index == kAdded || index == kDeleted) {
        size_t n = channels_.erase(fd);
        assert(n == 1);
        if(index == kAdded) {
            update(EPOLL_CTL_DEL , channel);
        }
        channel->set_index(kNew);
    }
}

void EPollPoller::update(int operation, Channel *channel) {     //epoll_ctl
    struct epoll_event event;
    bzero(&event , sizeof(event));
    event.events = channel->events();
    event.data.ptr = channel;
    int fd = channel->fd();
    LOG_INFO("%s %s %d : epoll_ctl_op=%s fd=%d event=%d ",debugline , operationToString(operation) , fd , channel->events());
    if(epoll_ctl(epollfd_ , operation , fd , &event)) {     //成功返回 0 , 失败返回 -1
        if(operation == EPOLL_CTL_DEL) {
            LOG_ERROR("%s %s %d : epoll_ctl_op=%s fd=%d errno=%d",debugline, operationToString(operation),fd,errno);
        } else {
            LOG_ERROR("%s %s %d : epoll_ctl_op=%s fd=%d errno=%d",debugline, operationToString(operation),fd,errno);
        }
    }
}

const char* EPollPoller::operationToString(int op) {
    switch (op)
    {
        case EPOLL_CTL_ADD:
            return "ADD";
        case EPOLL_CTL_DEL:
            return "DEL";
        case EPOLL_CTL_MOD:
            return "MOD";
        default:
            assert(false && "ERROR op");
            return "Unknown Operation";
    }
}

Timestamp EPollPoller::poll(int timeoutMs, Poller::ChannelList *activeChannels) {   //EventLoop -> EPollPoller::poll()将发生
    LOG_INFO("%s %s %d fd total count %zu",debugline,channels_.size());
    int numEvents = ::epoll_wait(epollfd_
                                ,&*events_.begin()
                                ,static_cast<int>(events_.size())
                                ,timeoutMs);
    int savaErrno = errno;
    Timestamp now(Timestamp::now());
    if(numEvents > 0) {
        LOG_INFO("%s %s %d : %d events happend",debugline,numEvents);
        fillActiveChannels(numEvents , activeChannels);     //将epoll_wait返回对应的channel填充到activeChannels中
        if(numEvents == events_.size()) {
            events_.resize(events_.size() * 2);
        }
    } else if (numEvents == 0){
        LOG_INFO("%s %s %d : nothing happend",debugline);
    } else {    //numEvents < 0     epoll_wait < 0
        if(savaErrno != EINTR) {
            errno = savaErrno;
            LOG_FATAL("%s %s %d : EPollPoller::poller",debugline);
        }
    }
    return now;
}

void EPollPoller::fillActiveChannels(int numEvents      //epoll_wait 返回值
                                     , Poller::ChannelList *activeChannels) const {
    assert(numEvents <= events_.size());
    for(int i=0 ; i < numEvents ;i++) {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        int fd = channel->fd();
        ChannelMap::const_iterator it = channels_.find(fd);
        assert(it != channels_.end());
        assert(it->second == channel);
        channel->set_revents(events_[i].events);
        activeChannels->emplace_back(channel);
    }
}



