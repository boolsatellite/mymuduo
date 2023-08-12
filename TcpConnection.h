#pragma once

#include "nocopyable.h"
#include "InetAddress.h"
#include "TimeStamp.h"
#include "Buffer.h"
#include "Callbacks.h"
#include <string>
#include <memory>
#include <atomic>

class Channel;
class EventLoop;
class Socket;

class TcpConnection : nocopyable
                    , public std::enable_shared_from_this<TcpConnection>
{
public: 
    TcpConnection(EventLoop* loop,              
                    const std::string& nameArg,
                    int sockfd,
                    const InetAddress& localAddr,
                    const InetAddress& peerAddr);

    ~TcpConnection();

    EventLoop* getLoop() const { return loop_; }
    const std::string& name() const { return name_; }
    const InetAddress& localAddress() const {return localAddr_; }
    const InetAddress& peerAddress() const {return peerAddr_; }

    bool connected() const {return state_ == kConnected; }
    bool disconected() const { return state_ == kDisconnected; }

    void shutdown();
    void shutdownInLoop();
    void send(const std::string& buf);

    //设置回调函数
    void setConnectionCallback(const ConnectionCallback& cb) { connetionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) {writeCompleteCallback_ = cb; }
    void setHighWaterMarkCallback(const HighWaterMarkCallback&cb) { highWaterCallback_ = cb; }
    void setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }

    void connectEstablished();   //连接建立了
    void connectDestroyed();     //连接销毁




private:
    enum StateE{ kDisconnected , kConnecting , kConnected , kDisconnecting }; 
    
    void setState(StateE state) { state_ = state; }

    void handleRead(TimeStamp receiveTiem);
    void handleWrite();
    void handleClose();
    void handleError();
    void sendInLoop(const void* data , size_t len);

    EventLoop* loop_;
    const std::string name_;
    std::atomic_int state_;
    bool reading_;
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    const InetAddress localAddr_;
    const InetAddress peerAddr_;
    
    ConnectionCallback connetionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    HighWaterMarkCallback highWaterCallback_;
    CloseCallback closeCallback_;

    size_t highWaterMark_;   //水位线
    Buffer inputBuffer_;     //接收数据缓冲区
    Buffer outputBuffer_;    //发送数据缓冲区
};