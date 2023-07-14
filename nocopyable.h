#pragma once

class nocopyable
{
public:
    nocopyable(const nocopyable&) = delete;
    nocopyable& operator=(const nocopyable&) = delete;

protected:
    nocopyable() = default;
    ~nocopyable() = default;
};

/*
1) public继承方式
基类中所有 public 成员在派生类中为 public 属性；
基类中所有 protected 成员在派生类中为 protected 属性；
基类中所有 private 成员在派生类中不能使用。

2) protected继承方式
基类中的所有 public 成员在派生类中为 protected 属性；
基类中的所有 protected 成员在派生类中为 protected 属性；
基类中的所有 private 成员在派生类中不能使用。

3) private继承方式
基类中的所有 public 成员在派生类中均为 private 属性；
基类中的所有 protected 成员在派生类中均为 private 属性；
基类中的所有 private 成员在派生类中不能使用。
*/