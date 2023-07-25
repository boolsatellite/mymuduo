#pragma once

#include "nocopyable.h"
#include "Thread.h"
#include <functional>
#include <string>
#include <mutex>
#include <condition_variable>

class EventLoop;

class EventLoopThread : nocopyable
{
public:
    using ThreadinitCallback = std::function<void(EventLoop*)>;

    EventLoopThread(const ThreadinitCallback& cb = ThreadinitCallback() , 
                        const std::string& name = std::string());
    ~EventLoopThread();

    EventLoop* startLoop();

private:
    void threadFunc();

    EventLoop* loop_;
    bool exiting_;
    Thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadinitCallback callback_;
};