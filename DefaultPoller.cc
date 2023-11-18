//
// Created by satellite on 11/11/2023.
//
#include "Poller.h"
#include "EPollPoller.h"

Poller *Poller::newDefaultPoller(EventLoop *loop) {
    return new EPollPoller(loop);
}