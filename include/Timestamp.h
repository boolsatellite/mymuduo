//
// Created by satellite on 2023-11-09.
//

#ifndef MYMUDUO_TIMESTAMP_H
#define MYMUDUO_TIMESTAMP_H


#include <cstdint>
#include <string>

class Timestamp {
public:
    Timestamp();
    explicit Timestamp(int64_t microSecondsSinceEpoch);
    static Timestamp now();
    std::string toString() const;
private:
    int64_t microSecondsSinceEpoch_;
};


#endif //MYMUDUO_TIMESTAMP_H
