//
// Created by satellite on 2023-11-10.
//

#include <sys/eventfd.h>
#include <functional>
#include <string>
#include <unistd.h>
#include <cassert>

#include "EventLoop.h"
#include "Logger.h"
#include "Channel.h"
#include "Poller.h"

//防止一个线程创建多个Eventloop,每个线程中用于一个loopInThisThread ,
// 为nullptr说明未当前线程没有分配EventLoop
__thread EventLoop *t_loopInThisThread = nullptr;

const int kPollTimeMs = 1000;   //epoll的默认超时时间

int createEventfd();


void EventLoop::printActiveChannels() const {
    for (const auto channel: activeChannels_) {
        LOG_INFO("{ channel->%s }", channel->reventsToString().c_str());
    }
}

EventLoop::EventLoop()
        : looping_(false), quit_(false), callingPendingFunctors_(false), threadId_(CurrentThread::tid()),
          poller_(Poller::newDefaultPoller(this)), wakeupFd_(createEventfd())                    //创建eventfd
        , wakeupChannel_(new Channel(this, wakeupFd_)) //将eventfd包装为Channel
        , currentActiveChannel_(nullptr) {
    LOG_INFO("%s %s %d : EventLoop created %p in thread %d ", debugline, this, threadId_);
    if (t_loopInThisThread != nullptr) {
        LOG_FATAL("%s %s %d : Another EventLoop %p in this thread %d",debugline , t_loopInThisThread, threadId_);
    } else {
        t_loopInThisThread = this;
    }
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));   //为 eventfd包装成的channel设置读回调
    wakeupChannel_->enableReading();                                            //使eventfd包装成的channel可读
    //eventfd管理的counter大于0时是可读的
}

void EventLoop::handleRead() {
    uint64_t one = 1;               // one 8字节
    ssize_t n = read(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG_ERROR("%s %s %d : handleRead read %ld instead of 8", debugline , n);
    }
}

EventLoop::~EventLoop() {
    LOG_INFO("%s %s %d : EventLoop %p of thread %d destructs in thread", debugline, this, CurrentThread::tid());
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop() {
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_ = false;
    LOG_INFO("EventLoop %p start looping", this);
    while (!quit_) {
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);

        for (Channel *channel: activeChannels_) {
            currentActiveChannel_ = channel;
            currentActiveChannel_->handleEvent(pollReturnTime_);
        }
        currentActiveChannel_ = nullptr;
        doPendingFunctors();        //mainLoop为subLoop注册function
    }
    LOG_INFO("EventLoop %p stop looping", this);
    looping_ = false;
}


void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }
    for (const Functor &functor: functors) {
        functor();
    }
    callingPendingFunctors_ = false;
}

void EventLoop::quit() {
    quit_ = true;
    if (!isInLoopThread()) {     //在其他线程调用当前线程中EventLoop::quit()
        wakeup();               //唤醒
        // todo:可以尝试不唤醒，EPollPoller::poll()超时会自动返回，找到对应对应代码位置
    }
}

void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one)) {
        LOG_ERROR("%s %s %d : writes %ld bytes instead of 8", debugline, n);
    }
}

void EventLoop::runInLoop(EventLoop::Functor cb) {
    if (isInLoopThread()) {
        cb();
    } else {
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(EventLoop::Functor cb) {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(cb);
    }
    if(!isInLoopThread() || callingPendingFunctors_) {
        //如果在其他线程内调用有要唤醒
        // 如果当前线程正在doPendingFunctors()，说明又注册了回调函数需要再次唤醒来处理注册的回调
        wakeup();
    }
}

void EventLoop::updateChannel(Channel *channel) {
    assert(channel->ownEventLoop() == this);
    assertInLoopThread();
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel) {
    assert(channel->ownEventLoop() == this);
    assertInLoopThread();
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel) {
    assertInLoopThread();
    poller_->removeChannel(channel);
    return poller_->hasChannel(channel);
}


int createEventfd() {
    int evfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evfd < 0) {
        LOG_FATAL("%s %s %d : eventfd error", debugline);
    }
    return evfd;
}
