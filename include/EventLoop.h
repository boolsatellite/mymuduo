//
// Created by satellite on 2023-11-10.
//

#ifndef MYMUDUO_EVENTLOOP_H
#define MYMUDUO_EVENTLOOP_H

#include <functional>
#include <vector>
#include <mutex>
#include <memory>
#include <atomic>

#include "nocopyable.h"
#include "Timestamp.h"
#include "CurrentThread.h"
#include "Logger.h"


class Channel;
class Poller;


// 包含 Channel 和 Poller
class EventLoop : nocopyable {
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    void loop();        //开始事件循环
    void quit();        //退出事件循环
    Timestamp pollReturnTime() { return pollReturnTime_; }

    void runInLoop(Functor cb);         //在当前loop中执行
    void queueInLoop(Functor cb);       //将cb放入队列中，唤醒loop所在的线程后执行

    void wakeup();                      //唤醒loop

    // 从此来调用 poller 的放饭
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);

    bool isInLoopThread() const {       //判断EventLoop是否再当前线程中
        return threadId_ == CurrentThread::tid();
    }

    void assertInLoopThread() {
        if(!isInLoopThread()) {
            LOG_FATAL("%s %s %d : EventLoop %p was created in threadId_ %d current thread id %d",
                      debugline , this , threadId_ , CurrentThread::tid());
        }
    }

private:

    //执行回调函数
    void handleRead();
    void doPendingFunctors();
    void printActiveChannels() const;

    using ChannelList = std::vector<Channel*>;
    std::atomic_bool looping_;
    std::atomic_bool quit_;      //标识退出loop
    std::atomic_bool callingPendingFunctors_;    //标识当前loop是否有需要执行的回调操作
    const pid_t threadId_;
    Timestamp pollReturnTime_;
    std::unique_ptr<Poller> poller_;

    int wakeupFd_;
    //当mainLoop获取一个新用户的channel，通过轮询算法选择一个subloop，通过该成员唤醒subloop处理channel
    std::unique_ptr<Channel> wakeupChannel_;        //用于唤醒的channel

    ChannelList activeChannels_;                    //Poller回填
    Channel* currentActiveChannel_;
    std::vector<Functor> pendingFunctors_ ;         //存储loop需要执行的回调操作
    std::mutex mutex_;                              //保护vector容器的线程安全操作




};

#endif //MYMUDUO_EVENTLOOP_H
