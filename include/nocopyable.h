//
// Created by satellite on 2023-11-08.
//

#ifndef MYMUDUO_NOCOPYABLE_H
#define MYMUDUO_NOCOPYABLE_H

class nocopyable {
public:
    nocopyable(const nocopyable &) = delete;    //不允许被拷贝,导致子类也不允许被拷贝
    nocopyable(const nocopyable &&) = delete;
    nocopyable& operator=(const nocopyable& ) = delete;
    nocopyable& operator=(const nocopyable && ) = delete;
protected:
    nocopyable() = default;         //允许被继承
    ~nocopyable() = default;
};


#endif //MYMUDUO_NOCOPYABLE_H
