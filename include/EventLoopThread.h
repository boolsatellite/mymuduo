//
// Created by satellite on 2023-11-21.
//

#ifndef MYMUDUO_EVENTLOOPTHREAD_H
#define MYMUDUO_EVENTLOOPTHREAD_H

#include <functional>
#include <condition_variable>
#include <mutex>
#include <string>

#include "nocopyable.h"
#include "Thread.h"

class EventLoop;

class EventLoopThread {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    explicit EventLoopThread( ThreadInitCallback  cb = ThreadInitCallback()
                    ,const std::string& name = std::string()) ;

    ~EventLoopThread();

    EventLoop* startLoop();
private:

    void threadFunc();

    EventLoop* loop_;
    bool exiting_;
    Thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallback callback_;
};


#endif //MYMUDUO_EVENTLOOPTHREAD_H
