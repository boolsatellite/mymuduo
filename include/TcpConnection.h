//
// Created by satellite on 2023-11-24.
//

#ifndef MYMUDUO_TCPCONNECTION_H
#define MYMUDUO_TCPCONNECTION_H

#include <memory>
#include <atomic>
#include "nocopyable.h"
#include "Callbacks.h"
#include "Timestamp.h"
#include "Buffer.h"
#include "InetAddress.h"


class Channel;
class EventLoop;
class Socket;


class TcpConnection : nocopyable ,
                      public std::enable_shared_from_this<TcpConnection> {

public:
    TcpConnection(EventLoop* loop ,
                  std::string  name ,
                  int sockfd ,
                  const InetAddress& localAddr ,
                  const InetAddress& peerAddr);
    ~TcpConnection();

    EventLoop* getLoop() const { return loop_; }
    const std::string& name() const { return name_; }
    const InetAddress& localAddress() const { return localAddr_; }
    const InetAddress& peerAddress() const { return peerAddr_; }
    bool connected() const { return state_ == kConnected; }
    bool disconnected() const { return state_ == kDisconnected; }

    void send(const void* message , int len);

    void shutdown();

    void setConnectionCallback(const ConnectionCallback& cb)
    { connectionCallback_ = cb; }

    void setMessageCallback(const MessageCallback& cb)
    { messageCallback_ = cb; }

    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    { writeCompleteCallback_ = cb; }

    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark)
    { highWaterMarkCallback_ = cb; highWaterMark_ = highWaterMark; }

    void setCloseCallback(const CloseCallback& cb)
    { closeCallback_ = cb; }

    Buffer* inputBuffer()
    { return &inputBuffer_; }

    Buffer* outputBuffer()
    { return &outputBuffer_; }

    void connectEstablished();      //连接建立
    void connectDestroyed();        //连接销毁


private:
    enum StateE { kDisconnected , kConnecting , kConnected , kDisconnecting };

    void setState(StateE s) { state_ = s; }

    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const void* message , size_t len);
    void shutdownInLoop();

    EventLoop* loop_;
    const std::string name_;
    std::atomic_int state_;
    bool reading_;

    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    const InetAddress localAddr_;
    const InetAddress peerAddr_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
    CloseCallback closeCallback_;

    size_t highWaterMark_;      //水位线
    Buffer inputBuffer_;        //接收数据的缓冲区
    Buffer outputBuffer_;       //发送数据的缓冲区
};


#endif //MYMUDUO_TCPCONNECTION_H
