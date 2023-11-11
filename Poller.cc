//
// Created by satellite on 11/11/2023.
//

#include "Poller.h"
#include "Channel.h"



Poller::Poller(EventLoop *loop) : ownerLoop_(loop) { }

Poller::~Poller() = default;

bool Poller::hasChannel(Channel *channel) {
    ChannelMap ::const_iterator it = channels_.find(channel->fd());
    return it != channels_.end() && it->second == channel;
}






