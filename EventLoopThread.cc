#include "EventLoopThread.h"
#include "EventLoop.h"
#include <memory>

EventLoopThread::EventLoopThread(const ThreadinitCallback &cb , const std::string &name )
                                   : loop_(NULL)
                                   , exiting_(false)
                                   , thread_(std::bind(&EventLoopThread::threadFunc,this),name)
                                   , mutex_()
                                   , cond_()
                                   , callback_(cb)
                                   {

                                   }



EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if(loop_ != nullptr)
    {
        loop_->quit();
        thread_.join();
    }
}

EventLoop *EventLoopThread::startLoop()
{
    thread_.start();

    EventLoop * loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while(loop_ == nullptr) 
        {
            cond_.wait(lock);
        }
        loop = loop_;
    }
    return loop;
}

//在单独的新线程内独立运行
void EventLoopThread::threadFunc()
{
    EventLoop loop;  //创建一个独立的eventloop和上面线程是一一对应的，one loop one thread
    if(callback_)
    {
        callback_(&loop);
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }
    loop.loop();    
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}