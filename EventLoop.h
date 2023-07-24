#pragma once

#include "nocopyable.h"
#include "TimeStamp.h"
#include "CurrentThread.h"
#include <functional>
#include <vector>
#include <atomic>
#include <mutex>
#include <memory>

class Channel;
class Poller;

//事件循环模块 包含 Channel Poller
class EventLoop : nocopyable
{
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    void loop();   //开启事件循环
    void quit();   //退出事件循环

    TimeStamp pollReturnTime() const { return pollReturnTime_; }    

    void runInLoop(Functor cb);   //在当前loop中执行cb
    void queueInLoop(Functor cb); //将cb放入队列中，唤醒loop所在的线程执行cb

    void wakeup();    //唤醒loop所在的线程

    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);

    //判断EventLoop对象是否在自己的线程里面
    bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }

private:
    void handleRead();           //weakup
    void doPendingFunctors();

    using ChannelList = std::vector<Channel*>;
    std::atomic_bool looping_;
    std::atomic_bool quit_;

    const pid_t threadId_;   //纪律当前loop所在线程id

    TimeStamp pollReturnTime_;
    std::unique_ptr<Poller> poller_;


    //主要作用，当mainLoop获取一个新用户的channel，
    //通过轮询算法选择一个subloop,通过该成员唤醒subloop
    int wakeupFd_;  
    std::unique_ptr<Channel> wakeupChannel_;

    ChannelList activeChannels_;

    std::vector<Functor> pendingFunctors_;  //存储loop需要执行的所有回调操作
    std::atomic_bool callingPendingFunctors_; //标识当前loop是否有需要执行的回调操作
    std::mutex mutex_;   //互斥锁，用来保护上面vector容器的线程安全操作

};