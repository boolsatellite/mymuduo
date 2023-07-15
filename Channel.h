#pragma once

#include "nocopyable.h"
#include "TimeStamp.h"
#include <functional>
#include <memory>

//Channel理解位通道，封装了sockfd和其感兴趣的event
//还绑定了Poller监听返回的具体事件

class EventLoop;

class Channel : nocopyable
{
public:
    using EventCallback = std::function<void()>;
    using ReadCallback = std::function<void(TimeStamp)>;
    
    Channel(EventLoop* loop , int fd);
    ~Channel();

    void handleEvent(TimeStamp receiveTime);  //在得到poller通知后执行根据发生的事件执行对应的回调函数

    //设置回调函数对象
    void setReadCallback(ReadCallback cb)
    {
        readCallback = std::move(cb);
    }
    void setWriteCallback(EventCallback cb)
    {
        writeCallback = std::move(cb);
    }
    void setCloseCallback(EventCallback cb)
    {
        closeCallback = std::move(cb);
    }
    void setErrorCallback(EventCallback cb)
    {
        errorCallback = std::move(cb);
    }

    //防止当channel被手动释放后，channel还在执行回调
    void tie(const std::shared_ptr<void>& obj);

    int fd() const { return fd_; }
    int events() const { return events_; }
    void set_revents(int revt) { revents_ = revt; }  //poller监听完对应的事件同时该接口设置revent

    //设置fd相应的事件状态
    void enableReading() { events_ |= KReadEvent; update(); }
    void disableReading() { events_ & ~KReadEvent; update(); }
    void enableWriting() { events_ |= KWriteEvent; update(); }
    void disableWriting() { events_ &= ~KWriteEvent; update(); }
    void disableAll() { events_ = KNoneEvent; }
    
    bool isWriting() const { return events_ & KWriteEvent; }
    bool isReading() const { return events_ & KReadEvent; }
    bool isNoneEvent() const { return events_ == KNoneEvent; }

    int index() { return index_; }
    void set_index(int idx) { index_ = idx; }

    EventLoop* ownerLoop() { return loop_; }
    void remove();

private:

    void update();  //更新poller中fd对应的事件
    void handleEventWithGuard(TimeStamp receiveTime);
    


    static const int KNoneEvent;
    static const int KReadEvent;
    static const int KWriteEvent;

    EventLoop * loop_;
    const int fd_;
    int events_;   //注册的事件
    int revents_;  //Poller返回发生的事件
    int index_;

    std::weak_ptr<void> tie_;
    bool tied_;
    
    //Channel中可以获取Poller中实际发生的事件，所有Channel负责执行对应的回调函数
    ReadCallback readCallback;
    EventCallback writeCallback;
    EventCallback closeCallback;
    EventCallback errorCallback;
};
