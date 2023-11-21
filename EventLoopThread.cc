//
// Created by satellite on 2023-11-21.
//

#include "EventLoopThread.h"
#include "EventLoop.h"

#include <cassert>

EventLoopThread::EventLoopThread(EventLoopThread::ThreadInitCallback cb
                                    , const std::string &name)
                                 : loop_(nullptr)
                                 , exiting_(false)
                                 , thread_(std::bind(&EventLoopThread::threadFunc , this) , name)
                                 , mutex_()
                                 , cond_()
                                 , callback_(std::move(cb))
{
}

EventLoopThread::~EventLoopThread() {
    exiting_ = true;
    if(loop_ != nullptr) {
        loop_->quit();
        thread_.join();
    }
}

EventLoop *EventLoopThread::startLoop() {
    assert(!thread_.started());
    thread_.start();        //开启线程执行 threadFunc
    EventLoop* loop = nullptr;
    {
         std::unique_lock<std::mutex> lock(mutex_);
         while(loop_ != nullptr) {
             cond_.wait(lock);      //解锁等待notify
         }
         loop = loop_;
    }
    return loop;
}

void EventLoopThread::threadFunc() {    //在单独的新线程内运行
    EventLoop loop;         //在新线程内创建一个EventLoop，one loop pre thread
    if(callback_) {
        callback_(&loop);   //调用线程初始化回调函数
    }
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }
    loop.loop();            //开启Poller

    //当可以执行到这里，说明poller::poll() 结束了，事件循环结束了
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}


