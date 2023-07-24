#pragma once

#include "Poller.h"
#include "TimeStamp.h"
#include <vector>
#include <sys/epoll.h>

class Channel;

class EPollPoller :public Poller
{
public:
    EPollPoller(EventLoop* loop);
    ~EPollPoller() override;

    virtual TimeStamp poll(int timeoutMs, ChannelList *activeChannels) override;
    virtual void updateChannel(Channel *channel) override;
    virtual void removeChannel(Channel *channel) override;

private:
    static const int kInitEventListSize = 16;

    void fillActiveChannels(int numEvents , ChannelList* activeChannels) const;
    void update(int operation , Channel * channel);

    using EventList = std::vector<struct epoll_event>;

    int epollfd_;
    EventList events_;
};