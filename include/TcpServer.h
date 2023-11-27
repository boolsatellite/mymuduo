//
// Created by satellite on 2023-11-10.
//

#ifndef MYMUDUO_TCPSERVER_H
#define MYMUDUO_TCPSERVER_H

#include "nocopyable.h"
#include "Callbacks.h"
#include "EventLoop.h"
#include "Acceptot.h"
#include "InetAddress.h"
#include "EventLoopThreadPool.h"

#include <string>
#include <atomic>
#include <unordered_map>

class TcpServer : nocopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    using ConnectionMap = std::unordered_map<std::string , TcpConnectionPtr>;

    enum Option { kNoReusePort , kReusePort };

    TcpServer(EventLoop* loop ,
              const InetAddress& listenAddr ,
              const std::string& nameArg ,
              Option option = kNoReusePort);
    ~TcpServer();

    const std::string& ipPort() const { return ipPort_; }
    const std::string& name() const { return name_; }
    EventLoop* getLoop() const { return loop_; }

    void setThreadNum(int numThreads);
    void setThreadInitCallback(const ThreadInitCallback& cb) { threadInitCallback_ = std::move(cb); }

    std::shared_ptr<EventLoopThreadPool> threadPool() { return threadPool_; }

    void start();

    void setConnectionCallback(const ConnectionCallback& cb)
    { connectionCallback_ = cb; }

    void setMessageCallback(const MessageCallback& cb)
    { messageCallback_ = cb; }

    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    { writeCompleteCallback_ = cb; }




private:

    /// Not thread safe, but in loop
    void newConnection(int sockfd, const InetAddress& peerAddr);
    /// Thread safe.
    void removeConnection(const TcpConnectionPtr& conn);
    /// Not thread safe, but in loop
    void removeConnectionInLoop(const TcpConnectionPtr& conn);


    EventLoop* loop_;           //用户定义的loop也就是baseloop
    const std::string ipPort_;
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_;                //主loop
    std::shared_ptr<EventLoopThreadPool> threadPool_;   //子loop

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    ThreadInitCallback threadInitCallback_;

    std::atomic_int started_;
    int nextConnId_;
    ConnectionMap connections_;         //保存所有的连接

};


#endif //MYMUDUO_TCPSERVER_H
