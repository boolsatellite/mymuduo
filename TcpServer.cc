//
// Created by satellite on 2023-11-10.
//

#include <cassert>
#include "TcpServer.h"
#include "TcpConnection.h"
#include "EventLoopThreadPool.h"

sockaddr_in getLocalAddr(int sockfd) {
    sockaddr_in addr;
    socklen_t len = sizeof(addr);
    if(getsockname(sockfd , reinterpret_cast<sockaddr *>(&addr), &len) < 0 ) {
        LOG_ERROR("getLocalAddr");
    }
    return addr;
}

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr,
                     const std::string &nameArg,
                     TcpServer::Option option)
                        : loop_(loop)
                        , ipPort_(listenAddr.toIpPort())
                        , name_(nameArg)
                        , acceptor_(new Acceptor(loop_,listenAddr,option))
                        , threadPool_(new EventLoopThreadPool(loop,nameArg))
                        , connectionCallback_(defaultConnectionCallback)
                        , messageCallback_(defaultMessageCallback)
                        , nextConnId_(1)
{
    //当有新连接时，调用TcpServer::newConnection
    acceptor_->setNewConnectionCallback([this](auto && PH1, auto && PH2) {
        newConnection(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2)); });
}

void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr) {
    loop_->assertInLoopThread();
    EventLoop* ioLoop = threadPool_->getNextLoop();     //轮询指向工作线程
    char buf[64];
    snprintf(buf , sizeof buf , "-%s#%d" , ipPort_.c_str() , nextConnId_);
    ++nextConnId_;
    std::string conName = name_ + buf;

    LOG_INFO("TcpServer::newConnection[%s]-new connection[%s] from %s"
             ,name_.c_str() , conName.c_str() , peerAddr.toIpPort().c_str());

    InetAddress localAddr(getLocalAddr(sockfd));

    TcpConnectionPtr conn(new TcpConnection(ioLoop ,
                                            conName ,
                                            sockfd ,
                                            localAddr ,
                                            peerAddr));
    connections_[conName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(
            [this](auto && PH1) { removeConnection(std::forward<decltype(PH1)>(PH1)); });
    ioLoop->runInLoop([conn] { conn->connectEstablished(); });
}

void TcpServer::setThreadNum(int numThreads) {
    assert(numThreads >=0);
    threadPool_->setThreadNum(numThreads);
}

void TcpServer::start() {
    if(started_++ == 0) {       //防止一个TcpServer对象被start多次
        threadPool_->start(threadInitCallback_);
        assert(!acceptor_->listening());
        loop_->runInLoop(
                std::bind(&Acceptor::listen , acceptor_.get()) );
    }
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn) {
    loop_->runInLoop([this, conn] { removeConnectionInLoop(conn); });
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn) {
    loop_->assertInLoopThread();
    LOG_INFO("TcpServer::removeConnectionInLoop [%s] - connection %s"
                ,name_.c_str() , conn->name().c_str());
    size_t n = connections_.erase(conn->name());
    assert(n == 1);
    EventLoop* ioLoop = conn->getLoop();    //获取conn所在的loop
    ioLoop->queueInLoop([conn] { conn->connectDestroyed(); });
}

TcpServer::~TcpServer() {
    loop_->assertInLoopThread();
    LOG_INFO(":~TcpServer() name %s ",name_.c_str());
    for(auto& item : connections_) {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop(
                [conn] { conn->connectDestroyed(); } );
    }
}

