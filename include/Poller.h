//
// Created by satellite on 11/11/2023.
//

#ifndef MYMUDUO_POLLER_H
#define MYMUDUO_POLLER_H

#include <functional>
#include <vector>
#include <unordered_map>

#include "nocopyable.h"
#include "Timestamp.h"

class EventLoop;
class Channel;

class Poller : nocopyable{
public:
    using ChannelList = std::vector<Channel*>;

    Poller(EventLoop* loop);
    virtual ~Poller();

    virtual Timestamp poll(int timeoutMs , ChannelList* activeChannels) = 0;
    virtual void updateChannel(Channel* channel) = 0;
    virtual void removeChannel(Channel* channel) =0 ;
    virtual bool hasChannel(Channel* channel) ;
    static Poller* newDefaultPoller(EventLoop* loop);




protected:
    //Map的key是sockfd ， value是对应的Channel*
    using ChannelMap = std::unordered_map<int , Channel*>;
    ChannelMap channels_;

private:
    EventLoop* ownerLoop_;
};


#endif //MYMUDUO_POLLER_H
