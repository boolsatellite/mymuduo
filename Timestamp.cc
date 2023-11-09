//
// Created by satellite on 2023-11-09.
//

#include <time.h>
#include "Timestamp.h"

Timestamp::Timestamp() : microSecondsSinceEpoch_(0) {

}

Timestamp::Timestamp(int64_t microSecondsSinceEpoch)
    :microSecondsSinceEpoch_(microSecondsSinceEpoch) {
}

Timestamp Timestamp::now() {
    time_t ti = time(NULL);
    return Timestamp(ti);
}

std::string Timestamp::toString() const {
    char buf[128];
    struct tm* t = localtime(&microSecondsSinceEpoch_);
    snprintf(buf , 128 , "%4d/%02d/%02d %02d:%02d:%02d" ,
             t->tm_year + 1900 ,
             t->tm_mon + 1 ,
             t->tm_wday  ,
             t->tm_hour  ,
             t->tm_min  ,
             t->tm_sec);
    return std::string (buf);
}



