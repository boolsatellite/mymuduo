#pragma once

#include "nocopyable.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "EventLoopThreadPool.h"
#include "InetAddress.h"
#include "Acceptor.h"
#include <functional>
#include "Callbacks.h"
#include <string>
#include <memory>
#include <atomic>
#include <unordered_map>

class TcpServer : nocopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    enum Option
    {
        kNReusePort,
        kReusePort
    };

    TcpServer(EventLoop* loop , const InetAddress& listenAddr , 
                const std::string& nameArg , Option option = kNReusePort);

    ~TcpServer();

    void setThreadNum(int numThreads);  //设置底层subloop的个数
    void setThreadInitCallback(const ThreadInitCallback& cb) { threadInitCallback_ = cb; }
    void setConnectionCallback(const ThreadInitCallback& cb) { threadInitCallback_ = cb; }
    void setMessageCallback(const ThreadInitCallback& cb) { threadInitCallback_ = cb; }
    void setWriteCompleteCallback(const ThreadInitCallback& cb) { threadInitCallback_ = cb; }
    
    void start();                      //开启服务器监听


private:
    using ConnectionMap = std::unordered_map<std::string , TcpConnectionPtr>;

    void newConnection(int sockfd , const InetAddress& peerAddr);
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn);


    EventLoop* loop_;   //baseLoop 用户定义的EventLoop
    const std::string ipPort_;
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_;
    std::shared_ptr<EventLoopThreadPool> threadPool_;
    
    ConnectionCallback connectionCallback_;       //有新连接请求时的回调
    MessageCallback messageCallback_;             //读写事件的回调
    WriteCompleteCallback writeCompleteCallback_; //消息写完后的回调

    ThreadInitCallback threadInitCallback_;       //线程初始化回调
    std::atomic_int started_;

    int nextConnId_;
    ConnectionMap connections_;               //保存所有的连接
};