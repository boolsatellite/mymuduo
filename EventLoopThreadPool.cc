//
// Created by satellite on 2023-11-22.
//

#include <cassert>
#include "EventLoopThreadPool.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseloop, const std::string &nameArg)
    : baseLoop_(baseloop)
    , name_(nameArg)
    , started_(false)
    , numThreads_(0)
    , next_(0)
{
}

void EventLoopThreadPool::start(const EventLoopThreadPool::ThreadInitCallback &cb) {
    assert(!started_);
    baseLoop_->assertInLoopThread();
    started_ = true;
    for(int i=0 ; i<numThreads_ ; ++i) {
        char buf[ name_.size() + 32];
        snprintf((char*)buf , sizeof buf , "%s%d" , name_.c_str() , i);
        EventLoopThread* t = (EventLoopThread*)new EventLoopThread(cb , buf);     //每个线程执行初始化函数
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        loops_.push_back(t->startLoop());       //创建EventLoop并返回对应地址
    }
    if(numThreads_ == 0 && cb) {
        cb(baseLoop_);
    }
}

EventLoop *EventLoopThreadPool::getNextLoop() {
    baseLoop_->assertInLoopThread();
    assert(started_);
    EventLoop* loop = baseLoop_;
    if(!loops_.empty()) {
        loop = loops_[next_];
        ++next_;
        if(next_ > loops_.size()) {
            next_ = 0;          //循环
        }
    }
    return loop;
}

EventLoop *EventLoopThreadPool::getloopForHash(size_t hashCode) {
    baseLoop_->assertInLoopThread();
    EventLoop* loop = baseLoop_;
    if(!loops_.empty()) {
        loop = loops_[hashCode % loops_.size()];
    }
    return loop;
}

std::vector<EventLoop *> EventLoopThreadPool::getAllLoops() {
    baseLoop_->assertInLoopThread();
    assert(started_);
    if (loops_.empty()) {
        return std::vector<EventLoop*> (1 , baseLoop_);
    } else {
        return loops_;
    }
}

EventLoopThreadPool::~EventLoopThreadPool() = default;


