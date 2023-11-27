//
// Created by satellite on 2023-11-23.
//

#include <cassert>
#include <unistd.h>
#include "Logger.h"
#include "Acceptot.h"
#include "Socket.h"
#include "EventLoop.h"
#include "InetAddress.h"

int createNoblockingOrDie() {
    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if(sockfd < 0) {
        LOG_FATAL("%s %s %d : Socket create error errno %d",debugline,errno);
    }
    return sockfd;
}

Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport)
                    : loop_(loop)
                    , acceptSocket_(createNoblockingOrDie())
                    , acceptChannel_(loop , acceptSocket_.fd())
                    , listenning_(false)
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(reuseport);
    acceptSocket_.bindAddress(listenAddr);
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead , this));
}

void Acceptor::handleRead() {       //acceptChannel的读回调
    loop_->assertInLoopThread();
    InetAddress peerAddr;
    int connfd = acceptSocket_.accept(&peerAddr);   //传出对端地址信息，返回连接socket
    if(connfd > 0) {
        if(newConnectionCallback_) {
            //用于轮询找到subLoop，唤醒分发Channel
            newConnectionCallback_(connfd , peerAddr);
        } else {
            ::close(connfd);
        }
    }
}

Acceptor::~Acceptor() {
    acceptChannel_.disableAll();
    acceptChannel_.remove();
}

void Acceptor::listen() {
    listenning_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();     //epoll_ctl
}




