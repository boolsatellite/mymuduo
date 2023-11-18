//
// Created by satellite on 2023-11-14.
//

#ifndef MYMUDUO_CURRENTTHREAD_H
#define MYMUDUO_CURRENTTHREAD_H

#include <cstdint>

namespace CurrentThread {
    extern __thread int t_cachedTid;
    extern __thread char t_tidString[32];
    extern __thread int t_tidStringLength;
    extern __thread const char * t_threadName;

    void cacheTid();
    void sleepUsec(int64_t usec);

    inline int tid() {
        if(__builtin_expect(t_cachedTid == 0 , 0)) {
            cacheTid();
        }
        return t_cachedTid;
    }

    inline const char* tidString() {
        return t_tidString;
    }

    inline int tidStringLength() {
        return t_tidStringLength;
    }

    inline const char * name() {
        return t_threadName;
    }

}

#endif //MYMUDUO_CURRENTTHREAD_H
