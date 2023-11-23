//
// Created by satellite on 2023-11-22.
//

#ifndef MYMUDUO_SOCKET_H
#define MYMUDUO_SOCKET_H


#include <bits/sockaddr.h>

class InetAddress;
struct tcp_info;


class Socket {
public:
    explicit Socket(int sockfd);
    ~Socket();

    int fd() const { return sockfd_; }

    bool getTcpInfo(struct tcp_info*) const ;
    bool getTcpInfoString(char* buf , int len) const ;
    void bindAddress(const InetAddress& localaddr);
    void listen();
    int accept(InetAddress* peeraddr);
    void shutdownWrite() const;
    void setTcpNoDelay(bool on) const;
    void setReuseAddr(bool on) const;
    void setReusePort(bool on) const;
    void setKeepAlive(bool on) const;

private:
    const int sockfd_;

};


#endif //MYMUDUO_SOCKET_H
