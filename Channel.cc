//
// Created by satellite on 2023-11-10.
//

#include <sys/epoll.h>
#include <cassert>
#include <sstream>
#include <iostream>
#include "Channel.h"
#include "EventLoop.h"
#include "Logger.h"

const int Channel::kNoneEvent = 0;
const int Channel::kWriteEvent = EPOLLOUT;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;


Channel::Channel(EventLoop *loop, int fd)
        : loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1), tied_(false) {
}

Channel::~Channel() {                 //todo 看源码中的断言，暂时没理解
    //if (loop_->isInLoopThread()) {
    //    assert(!loop_->hasChannel(this));
    //}
}

void Channel::tie(const std::shared_ptr<void> &obj) {   //tie观察传入的share_ptr
    tie_ = obj;
    tied_ = true;
}

void Channel::update() {
    loop_->updateChannel(this);
}

void Channel::remove() {
    loop_->removeChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime) {
    std::shared_ptr<void> guard;
    if(tied_) {
        guard = tie_.lock();        //保证channel没有被释放
        handleEventWithGuard(receiveTime);
    } else {
        handleEventWithGuard(receiveTime);
    }
}

void Channel::handleEventWithGuard(Timestamp receiveTime) {
    LOG_INFO("%s:%s:%d | channel handleEvent revent:%d",debugline ,revents_);
    if( (revents_ & EPOLLHUP) && (revents_ & EPOLLIN)) {    //发生异常
        if(closeCallback_) {
            closeCallback_();
        }
    }

    if(revents_ & EPOLLERR ) {
        if(errorCallback_) {
            errorCallback_();
        }
    }

    if(revents_ & (EPOLLIN | EPOLLPRI)) {       //有可读事件或带外数据
        if(readCallback_) {
            readCallback_(receiveTime);
        }
    }

    if(revents_ & EPOLLOUT) {               //有可写事件
        if(writeCallback_) {
            writeCallback_();
        }
    }
}


std::string Channel::reventsToString() const {
    return Channel::eventsToString(fd_, revents_);
}

std::string Channel::eventsToString() const {
    return Channel::eventsToString(fd_, events_);
}


std::string Channel::eventsToString(int fd, int ev) const {
    std::ostringstream oss;
    oss << fd << ": ";
    if (ev & EPOLLIN)
        oss << "IN ";
    if (ev & EPOLLPRI)
        oss << "PRI ";
    if (ev & EPOLLOUT)
        oss << "OUT ";
    if (ev & EPOLLHUP)
        oss << "HUP ";
    if (ev & EPOLLRDHUP)
        oss << "RDHUP ";
    if (ev & EPOLLERR)
        oss << "ERR ";

    return oss.str();
}


//tie() 是怎样被触发的？
