//
// Created by satellite on 2023-11-18.
//

#include "Thread.h"
#include "CurrentThread.h"
#include <utility>
#include <semaphore.h>


std::atomic_int Thread::numCreated_ {0};      //类静态成员，记录产生线程的个数

Thread::Thread(Thread::ThreadFunc func, std::string name = std::string())
        : started_(false)
        , joined_(false)
        , tid_(0)
        , func_(std::move(func))
        , name_(std::move(name))
{
    setDefaultName();       //导致 static numCreate++
}

Thread::~Thread() {
    if(started_ && !joined_) {
        thread_->detach();
    }
}

void Thread::start() {

    sem_t sem;
    sem_init(&sem , false , 0);

    thread_ = std::make_shared<std::thread>([&](){
        tid_ = CurrentThread::tid();    //获取新开启线程的tid
        sem_post(&sem);
        func_();

    });
    //这里必须等待上面新创建的线程的tid值(用于标识线程成功创建)，采用信号量解决
    sem_wait(&sem);
    sem_destroy(&sem);
}

void Thread::join() {
    joined_ = true;
    thread_->join();
}

void Thread::setDefaultName() {
    int num = ++this->numCreated_;
    if(name_.empty()) {
        char buf[32];
        snprintf(buf , sizeof buf , "Thread%d",num);
        name_ = buf;
    }
}

