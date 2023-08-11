#pragma once

#include "nocopyable.h"

class InetAddress;

class Socket : nocopyable
{
public:
    explicit Socket(int sockfd)
        :sockfd_(sockfd)
    { }

    ~Socket();

    int fd() const { return sockfd_; }
    void bindAddress(const InetAddress& localaddr);
    void listen();
    int accept(InetAddress* peeraddr);

    void shutdownWrite();

    void setTcpNoDelay(bool on);  //设置TCP无延迟
    void setReuseAddr(bool on);   //设置地址重用
    void setReusePort(bool on);   //设置端口重用
    void setKeepAlive(bool on);

private:
    const int sockfd_;
};

