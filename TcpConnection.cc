//
// Created by satellite on 2023-11-24.
//

#include "TcpConnection.h"

#include <utility>
#include <cassert>
#include <unistd.h>
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Logger.h"

TcpConnection::TcpConnection(EventLoop *loop, std::string name, int sockfd
                             , const InetAddress &localAddr
                             , const InetAddress &peerAddr)
        : loop_(loop)
        , name_(std::move(name))
        , state_(kConnecting)
        , reading_(true)
        , socket_(new Socket(sockfd))
        , channel_(new Channel(loop,sockfd))
        , localAddr_(localAddr)
        , peerAddr_(peerAddr)
        , highWaterMark_(64 * 1024 * 1024)
{
     channel_->setReadCallback(
             std::bind(&TcpConnection::handleRead , this , std::placeholders::_1));
     channel_->setWriteCallback(
             std::bind(&TcpConnection::handleWrite , this));
     channel_->setCloseCallback(
             std::bind(&TcpConnection::handleClose , this));
     channel_->setErrorCallback(
             std::bind(&TcpConnection::handleError , this));
    LOG_INFO("TcpConnection::ctor %s at %p fd = %d",name_.c_str() , this , sockfd);
    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection() {
    LOG_INFO("TcpConnection::dtor %s at %p fd = %d",name_.c_str() , this , channel_->fd());
    assert(state_ == kDisconnected);
}

void TcpConnection::handleRead(Timestamp receiveTime) {
    loop_->assertInLoopThread();
    int saveErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd() , &saveErrno);
    if(n > 0) {
        messageCallback_(shared_from_this() , &inputBuffer_ , receiveTime);
    } else if(n == 0) {
        handleClose();
    } else {
        errno = saveErrno;
        LOG_ERROR("%s %s %d : handleRead errno %d ",debugline, errno);
        handleError();
    }
}

void TcpConnection::handleWrite() {
    loop_->assertInLoopThread();
    if(channel_->isWriting()) {
        ssize_t n = ::send(channel_->fd() , outputBuffer_.peek()
                         , outputBuffer_.readableBytes() , 0);
        if(n > 0) {
            outputBuffer_.retrieve(n);
            if(outputBuffer_.readableBytes() == 0) {
                channel_->disableWriting(); //没用数据可以发送时将channel设置为对写不感兴趣
                if(writeCompleteCallback_) {
                    loop_->queueInLoop(std::bind(writeCompleteCallback_ , shared_from_this()));
                }
                if(state_ == kDisconnecting) {
                    shutdownInLoop();
                }
            }
        } else {
            LOG_ERROR("%s %s %d",debugline);
        }
    } else {
        LOG_ERROR("%s %s %d : Connection fd=%d is down , no more writing",debugline , channel_->fd());
    }
}

void TcpConnection::handleClose() {
    loop_->assertInLoopThread();
    LOG_INFO("fd = %d state = %d",channel_->fd() , static_cast<int>(state_));
    assert(state_ == kConnected || state_ == kDisconnecting);
    setState(kDisconnected);
    //不close fd ，这应该由析构函数来做，即智能指针释放
    channel_->disableAll();
    TcpConnectionPtr guardThis(shared_from_this());
    if(connectionCallback_)
        connectionCallback_(guardThis);
    if(closeCallback_)
        closeCallback_(guardThis);
}

void TcpConnection::handleError() {
    int optval;
    int err;
    socklen_t oplen = sizeof optval;
    if(::getsockopt(channel_->fd() , SOL_SOCKET , SO_ERROR , &optval , &oplen) < 0) {
        err = errno;
    } else {
        err = optval;
    }
    LOG_ERROR("TcpConnection::handleError SO_ERROR %d ",err);
}

void TcpConnection::send(const void *message, int len) {
    if(state_ == kConnected) {
        if(loop_->isInLoopThread()) {
            sendInLoop(message , len);
        } else {
            loop_->queueInLoop(
                    [this, message, len] { sendInLoop(message, len); }
                    );
        }
    }
}

void TcpConnection::sendInLoop(const void *message, size_t len) {
    loop_->assertInLoopThread();
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;
    if(state_ == kDisconnected) {
        LOG_ERROR("%s %s %d : disconnected give up writing",debugline);
        return;
    }
    //如歌输出队列中没有数据尝试写入内核缓冲
    if(!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
        nwrote = ::send(channel_->fd() , message , len , 0);
        if(nwrote >= 0) {
            remaining = len - nwrote;
            if(remaining == 0 && writeCompleteCallback_) {
                //尽然一次将数据发送完成了，也就不需要为channel设置EPOLL_OUT事件感兴趣了，handwrite没有必要执行
                loop_->queueInLoop(std::bind(writeCompleteCallback_,shared_from_this()));
            }
        } else {        //nwrote < 0
            nwrote = 0;
            if(errno != EWOULDBLOCK) {
                LOG_ERROR("%s %s %d : send error",debugline);
                if(errno == EPIPE || errno == ECONNRESET) {
                    faultError = true;
                }
            }
        }
    }

    if (!faultError && remaining > 0) {     //没有出错，并且数据在此之前没有一次性发送完成
        size_t oldlen = outputBuffer_.readableBytes();
        if(oldlen + remaining >= highWaterMark_
                && oldlen < highWaterMark_
                && highWaterMarkCallback_) {
            loop_->queueInLoop(std::bind(highWaterMarkCallback_,shared_from_this(),oldlen+remaining));
        }
        outputBuffer_.append(static_cast<const char*>(message)+nwrote , remaining);       //将数据放入输出缓冲
        if(!channel_->isWriting()) {
            channel_->enableWriting();                          //设置可写
        }
    }
}

void TcpConnection::connectEstablished() {
    loop_->assertInLoopThread();
    assert(state_ == kConnecting);
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading();
    connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed() {
    loop_->assertInLoopThread();
    if(state_ == kConnected) {
        setState(kDisconnected);
        channel_->disableAll();
        connectionCallback_(shared_from_this());
    }
    channel_->remove();
}

void TcpConnection::shutdown() {
    if(state_ == kConnected) {
        setState(kDisconnecting);
        loop_->runInLoop([this] { shutdownInLoop(); });
    }
}

void TcpConnection::shutdownInLoop() {
    loop_->assertInLoopThread();
    if(!channel_->isWriting()) {
        socket_->shutdownWrite();
        //::close(channel_->fd());
    }
}

