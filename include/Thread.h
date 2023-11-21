//
// Created by satellite on 2023-11-18.
//

#ifndef MYMUDUO_THREAD_H
#define MYMUDUO_THREAD_H

#include <thread>
#include <functional>
#include <memory>
#include <unistd.h>
#include <string>
#include <atomic>

#include "nocopyable.h"


class Thread : nocopyable {
public:

    using ThreadFunc = std::function<void()>;

    explicit Thread(ThreadFunc func , std::string  name);
    ~Thread();

    void start();
    void join();

    bool started() const { return started_; }
    pid_t tid() const { return tid_;}
    const std::string& name() const { return name_; }

    void setDefaultName();

    static int numCreated() { return numCreated_; }


private:

    bool started_;
    bool joined_;
    std::shared_ptr<std::thread> thread_;
    pid_t tid_;
    ThreadFunc func_;
    std::string  name_;
    static std::atomic_int numCreated_;      //类静态成员，记录产生线程的个数

};


#endif //MYMUDUO_THREAD_H

