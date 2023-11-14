//
// Created by satellite on 11/11/2023.
//

#ifndef MYMUDUO_EPOLLPOLLER_H
#define MYMUDUO_EPOLLPOLLER_H

#include <vector>
#include <sys/epoll.h>

#include "Poller.h"
#include "EventLoop.h"


class EPollPoller : public Poller {
public:
    explicit EPollPoller(EventLoop *loop);                                  //epoll_create
    ~EPollPoller() override;                                                //epoll_close

    //基类方法的重写
    Timestamp poll(int timeoutMs, ChannelList *activeChannels) override;    //epoll_wait
    void updateChannel(Channel *channel) override;                          //epoll_ctl
    void removeChannel(Channel *channel) override;

    const char* operationToString(int op);
private:
    static const int kInitEventListSize = 16;

    //填写活跃的channel
    void fillActiveChannels(int numEvents , ChannelList* activeChannels) const ;
    //更新channel
    void update(int operation , Channel* channel);

    using EventList = std::vector<struct epoll_event> ;

    int epollfd_;
    EventList events_;
};


#endif //MYMUDUO_EPOLLPOLLER_H
