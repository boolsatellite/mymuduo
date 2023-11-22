//
// Created by satellite on 2023-11-22.
//

#ifndef MYMUDUO_EVENTLOOPTHREADPOOL_H
#define MYMUDUO_EVENTLOOPTHREADPOOL_H

#include "nocopyable.h"
#include "EventLoop.h"
#include "EventLoopThread.h"

#include <string>
#include <functional>

class EventLoopThreadPool : nocopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThreadPool(EventLoop* baseloop , const std::string& nameArg);
    ~EventLoopThreadPool();
    void setThreadNum(int numThreads) { numThreads_ = numThreads; }
    void start(const ThreadInitCallback& cb = ThreadInitCallback());

    EventLoop* getNextLoop();   //baseLoop默认以轮询的方式分配channel给subLoop
    EventLoop* getloopForHash(size_t hashCode);

    std::vector<EventLoop*> getAllLoops();

    bool started() const { return started_; }
    const std::string& name() const { return name_; }

private:
    EventLoop* baseLoop_;
    std::string name_;
    bool started_;
    int numThreads_;
    int next_;              //采用轮询的体现
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;
};


#endif //MYMUDUO_EVENTLOOPTHREADPOOL_H
