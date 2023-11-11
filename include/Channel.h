//
// Created by satellite on 2023-11-10.
//

#ifndef MYMUDUO_CHANNEL_H
#define MYMUDUO_CHANNEL_H

#include <functional>
#include <memory>
#include "nocopyable.h"
#include "Timestamp.h"

class EventLoop;

class Channel : nocopyable {
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    Channel(EventLoop* loop , int fd);
    ~Channel();

    void handleEvent(Timestamp receiveTime);

    //为channel设置回调
    void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); }
    void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
    void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

    int fd() const { return fd_; }
    int events() const { return events_; }
    int set_revents(int rect) { revents_ = rect; }

    //判断感兴趣的事件
    bool isNoneEvent() const { return events_ == kNoneEvent; }
    bool isWriting() const { return events_ & kWriteEvent; }
    bool isReading() const { return events_ & kReadEvent}

    //设置感兴趣的事件
    void enableReading() { events_ |= kReadEvent; update(); }
    void disableReading() { events_ &= ~kReadEvent; update();}
    void enableWriting() { events_ |= kWriteEvent; update(); }
    void disableWriting() { events_ &= ~kWriteEvent; update();}
    void disableAll() { events_ = kNoneEvent; update()}

    EventLoop* ownEventLoop() { return loop_; }

    void tie(const std::shared_ptr<void>& );
    void remove();


private:

    void update();
    void handleEventWithGuard(Timestamp receiveTime);

    static const int kNoneEvent;        //不对任何事件感兴趣
    static const int kReadEvent;        //对读事件感兴趣
    static const int kWriteEvent;       //对写事件感兴趣

    EventLoop* loop_;                   //当前Channel属于的事件循环
    const int fd_;
    int events_;                        //当前channel感兴趣的事件
    int revents_;                       //poller返回具体发生的事件
    int index_;

    std::weak_ptr<void> tie_;
    bool tied_;

    //Channel可以通过poller获知fd最终发生的具体事件revents，所以他负责执行具体回调
    ReadEventCallback   readCallback_;
    EventCallback  writeCallback_;
    EventCallback  closeCallback_;
    EventCallback  errorCallback_;
};


#endif //MYMUDUO_CHANNEL_H
