#pragma once
#include "nocopyable.h"
#include "TimeStamp.h"
#include <vector>
#include <unordered_map>


class Channel;
class EventLoop;

class Poller
{
public:
    using ChannelList = std::vector<Channel*>;
    Poller(EventLoop* loop);
    virtual ~Poller();

    //给所有IO复用保留统一的接口
    virtual TimeStamp poll(int timeoutMs , ChannelList* activeChannels) = 0;
    virtual void updateChannel(Channel * channel);
    virtual void removeChannel(Channel * channel);
    
    bool hasChannel(Channel * channel) const;   //判断当前poller中是否拥有某个channel

    static Poller* newdefautPoller(EventLoop * loop);  //EventLoop可以通过该接口获取默认的IO复用的具体实现

protected:
    //map的key是fd ， valute是fd所属的Channel
    using ChannelMap = std::unordered_map<int,Channel*>;
    ChannelMap channels_;

private:
    EventLoop* ownLoop_;
};