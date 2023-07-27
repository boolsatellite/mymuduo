#include "Socket.h"
#include "InetAddress.h"
#include "Logger.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/tcp.h>

Socket::~Socket()
{
    close(sockfd_);
}


void Socket::bindAddress(const InetAddress &localaddr)
{
    if(::bind(sockfd_,(struct sockaddr*)localaddr.getSockAddr(),sizeof(struct sockaddr) < 0 ))
    {
        LOG_FATAL("bind sockfd %d fail \n",sockfd_); 
    } 
}

void Socket::listen()
{
    if((::listen(sockfd_,1024)) < 0)
    {
        LOG_FATAL("listen sockfd : %d fail \n",sockfd_);
    }

}
int Socket::accept(InetAddress *peeraddr)
{
    struct sockaddr_in addr;
    bzero(&addr,sizeof(sockaddr_in));
    socklen_t len = sizeof(addr);
    int connfd = ::accept(sockfd_,(struct sockaddr*)&addr,&len);
    if(connfd > 0)
    {
        peeraddr->setSockAddrInet(addr);
    }
    return connfd;
}

void Socket::shutdownWrite()
{
    if(::shutdown(sockfd_,SHUT_WR) < 0)
    {
        LOG_ERROR("shutdownWrite error \n");
    }
}

void Socket::setTcpNoDelay(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_ , IPPROTO_TCP , TCP_NODELAY,
                &optval , static_cast<socklen_t>(sizeof optval));
}

void Socket::setReuseAddr(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_ , SOL_SOCKET, SO_REUSEADDR,
                &optval , static_cast<socklen_t>(sizeof optval));
}
void Socket::setReusePort(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_ , SOL_SOCKET, SO_REUSEPORT,
                &optval , static_cast<socklen_t>(sizeof optval));

}
void Socket::setKeepAlive(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_ , SOL_SOCKET, SO_KEEPALIVE,
                &optval , static_cast<socklen_t>(sizeof optval));
}