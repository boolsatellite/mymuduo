//
// Created by satellite on 2023-11-22.
//

#include "Socket.h"
#include "Logger.h"
#include "InetAddress.h"
#include <unistd.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <cstring>



Socket::Socket(int sockfd)
    :sockfd_(sockfd)
{}

Socket::~Socket() {
    close(sockfd_);
}

bool Socket::getTcpInfo(struct tcp_info *tcpi) const {
    socklen_t len = sizeof (*tcpi);
    bzero(tcpi , len);
    return ::getsockopt(sockfd_ , SOL_TCP , TCP_INFO , tcpi , &len) == 0;
}

bool Socket::getTcpInfoString(char* buf, int len) const {
    struct tcp_info tcpi;
    bool ok = getTcpInfo(&tcpi);
    if (ok)
    {
        snprintf(buf, len, "unrecovered=%u "
                           "rto=%u ato=%u snd_mss=%u rcv_mss=%u "
                           "lost=%u retrans=%u rtt=%u rttvar=%u "
                           "sshthresh=%u cwnd=%u total_retrans=%u",
                 tcpi.tcpi_retransmits,  // Number of unrecovered [RTO] timeouts
                 tcpi.tcpi_rto,          // Retransmit timeout in usec
                 tcpi.tcpi_ato,          // Predicted tick of soft clock in usec
                 tcpi.tcpi_snd_mss,
                 tcpi.tcpi_rcv_mss,
                 tcpi.tcpi_lost,         // Lost packets
                 tcpi.tcpi_retrans,      // Retransmitted packets out
                 tcpi.tcpi_rtt,          // Smoothed round trip time in usec
                 tcpi.tcpi_rttvar,       // Medium deviation
                 tcpi.tcpi_snd_ssthresh,
                 tcpi.tcpi_snd_cwnd,
                 tcpi.tcpi_total_retrans);  // Total retransmits for entire connection
    }
    return ok;
}

void Socket::bindAddress(const InetAddress &localaddr) {
    int ret = bind(sockfd_ ,localaddr.getSockAddr() , sizeof(sockaddr) );
    if(ret < 0) {
        LOG_FATAL("%s %s %d : socket bind error",debugline);
    }
}

void Socket::listen() {
    int ret = ::listen(sockfd_ , SOMAXCONN);
    if(ret < 0) {
        LOG_FATAL("%s %s %d : socket listen error",debugline);
    }
}

int Socket::accept(InetAddress *peeraddr) {         //peeraddr传出参数，保存对端信息
    socklen_t addrlen = sizeof(peeraddr->getSockAddr());
    int connfd = ::accept(sockfd_ , const_cast<sockaddr*>(peeraddr->getSockAddr()) , &addrlen);
    if (connfd < 0) {
        int savedErrno = errno;
        //LOG_SYSERR << "Socket::accept";
        LOG_ERROR("Socket::accept");
        switch (savedErrno) {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO: // ???
            case EPERM:
            case EMFILE: // per-process lmit of open file desctiptor ???
                // expected errors
                errno = savedErrno;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                // unexpected errors
                //LOG_FATAL << "unexpected error of ::accept " << savedErrno;
                LOG_FATAL("unexpected error of ::accept %d",savedErrno);
                break;
            default:
                //LOG_FATAL << "unknown error of ::accept " << savedErrno;
                LOG_FATAL("unknown error of ::accept %d",savedErrno);
                break;
        }
    }
    return connfd;
}

void Socket::shutdownWrite() const {
    if(::shutdown(sockfd_ , SHUT_WR) < 0) {
        LOG_ERROR("Socket::shutdownWrite");
    }
}

void Socket::setTcpNoDelay(bool on) const {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY,
                 &optval, static_cast<socklen_t>(sizeof optval));
}

void Socket::setReuseAddr(bool on) const {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_ , SOL_SOCKET , SO_REUSEADDR , &optval , sizeof optval);
}

void Socket::setReusePort(bool on) const {
#ifdef SO_REUSEPORT
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT,
                           &optval, static_cast<socklen_t>(sizeof optval));
    if (ret < 0 && on)
    {
        LOG_ERROR("SO_REUSEPORT failed");
    }
#else
    if (on)
  {
    LOG_ERROR << "SO_REUSEPORT is not supported.";
  }
#endif
}

void Socket::setKeepAlive(bool on) const {
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE,
                 &optval, static_cast<socklen_t>(sizeof optval));
}