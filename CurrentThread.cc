//
// Created by satellite on 2023-11-14.
//

#include <cstdio>
#include <csignal>
#include <ctime>
#include "CurrentThread.h"
#include "unistd.h"
#include "sys/syscall.h"
#include "Timestamp.h"


namespace CurrentThread {
    __thread int t_cachedTid;
    __thread char t_tidString[32];
    __thread int t_tidStringLength = 6;
    __thread const char * t_threadName = "unknown";

    void cacheTid() {
        if(t_cachedTid == 0) {
            t_cachedTid = static_cast<pid_t>(syscall(SYS_gettid));
            t_tidStringLength = snprintf(t_tidString,sizeof t_tidString,"%5d ",t_cachedTid);
        }
    }

    void sleepUsec(int64_t usec) {
        struct timespec ts = {0,0};
        ts.tv_sec = static_cast<time_t>(usec/Timestamp::kMicroSecondsPerSecond);
        ts.tv_nsec = static_cast<long>(usec % Timestamp::kMicroSecondsPerSecond);
        ::nanosleep(&ts , NULL);
    }

}