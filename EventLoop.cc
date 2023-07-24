#include "EventLoop.h"
#include "Logger.h"
#include "Channel.h"
#include "Poller.h"
#include <sys/eventfd.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <memory>

// 防止一个线程创建多个EventLoop对象，
// 当创建一个EventLoop对象时就让t_loopInThisThread指向对应的EventLoop
__thread EventLoop *t_loopInThisThread = nullptr;

const int kPollTimeMs = 1000; // 定义默认poller IO复用接口的超时时间

// 创建wakeupfd，用来处理notify唤醒subReactor处理新来的channel
int createEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
    {
        LOG_FATAL("eventfd errnor : %d \n", errno);
        abort();
    }
    return evtfd;
}

EventLoop::EventLoop()
    : looping_(false)
    , quit_(false)
    , callingPendingFunctors_(false)
    , threadId_(CurrentThread::tid())
    , poller_(Poller::newdefautPoller(this))
    , wakeupFd_(createEventfd())
    , wakeupChannel_(new Channel(this, wakeupFd_))
{
    LOG_DEBUG("EventLoop created : %p in thread %d \n", this, threadId_);
    if (t_loopInThisThread)
    {
        LOG_FATAL("Another EventLoop %p exists in this thread %d \n", this, threadId_);
    }
    else
    {
        t_loopInThisThread = this;
    }

    // 设置wakeupfd的事件类型以及发生事件后的回调操作
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    // 每一个EventLoop都将监听wakeupChannel中的EPOLLiN读事件了
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
    wakeupChannel_->disableAll();   //对任何事件不感兴趣
    wakeupChannel_->remove();       //将channel从poller中删除
    close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one))
    {
        LOG_ERROR("EventLoop::handleRead() reads %ld bytes instead of 8", n);
    }
}

void EventLoop::loop()
{
    looping_ = true;
    quit_ = false;
    LOG_INFO("EventLoop:%p start looping \n",this);
    while (!quit_)  
    {
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMs,&activeChannels_);
        for(Channel* channel : activeChannels_)
        {
            //poller监听那些channel发生变化了上报Eventloop,通知channel处理相应事件
            channel->handleEvent(pollReturnTime_);
        }
        //执行当前EventLoop事件循环需要处理的回调操作
        doPendingFunctors();
    }
    LOG_INFO("EventLoop %p stop looping \n",this);
    looping_ = false;
}

void EventLoop::quit()
{
    quit_ = true;
    if(!isInLoopThread())
    {
        wakeup();
    }
}

void EventLoop::runInLoop(Functor cb)
{
    if(isInLoopThread())   //在当前loop线程中执行cb
    {
        cb();
    }
    else      //在非当前线程中共执行cb，需要唤醒loop所在的线程执行cb
    {
        queueInLoop(std::move(cb));
    }
}

void EventLoop::queueInLoop(Functor cb)
{
    {
        std::unique_lock<std::mutex>(mutex_);
        pendingFunctors_.emplace_back(cb);
    }
    //唤醒相应需要执行上面回调操作的loop线程
    if(!isInLoopThread() || callingPendingFunctors_) 
    {
        wakeup();
    }
}

//用来唤醒loop所在的线程 向wakeupfd写一个数据
//导致wakeupchannel有读事件发生，当前loop就会被唤醒
void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_,&one,sizeof(one));
    if(n != sizeof(one))
    {
        LOG_FATAL("EventLoop::wakeup() write %lu byutes instead of 8",n);
    }
}

void EventLoop::updateChannel(Channel * channel)
{
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel * channel)
{
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel * channel)
{
    return poller_->hasChannel(channel);
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    {
        std::unique_lock<std::mutex>(mutex_);
        functors.swap(pendingFunctors_);
    }
    for(const Functor& functor : functors)
    {
        functor();   //执行当前loop需要执行的回调操作
    }
    callingPendingFunctors_ = false;
}