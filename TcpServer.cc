#include "TcpServer.h"
#include "Logger.h"
#include "TcpConnection.h"
#include <functional>
#include <strings.h>



static EventLoop* CheckLoopNotNull(EventLoop* loop)
{
    if(loop == nullptr)
    {
        LOG_FATAL("%s:%s:%d mainLoop is null!!! \n",__FILE__,__FUNCTION__,__LINE__);
    }
    return loop;
}

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr,
                     const std::string &nameArg, Option option)
                        :loop_(CheckLoopNotNull(loop))
                        ,ipPort_(listenAddr.toIpPort())
                        ,name_(nameArg)
                        ,acceptor_(new Acceptor(loop,listenAddr,option == kNReusePort))
                        ,threadPool_(new EventLoopThreadPool(loop,name_))
                        ,connectionCallback_()
                        ,messageCallback_()
                        ,nextConnId_(1)
{
    //当用户连接时会执行TcpServer::NewConnectionCallback回调操作
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection,this,std::placeholders::_1,std::placeholders::_2));
}

TcpServer::~TcpServer()
{
    for(auto & item : connections_)
    {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop(
            std::bind(&TcpConnection::connectDestroyed , conn)
        );
    }
}

void TcpServer::setThreadNum(int numThreads)
{
    threadPool_->setThreadNum(numThreads);
}

void TcpServer::start()
{
    if(started_++ == 0)
    {
        threadPool_->start(threadInitCallback_);
        loop_->runInLoop(std::bind(&Acceptor::listen,acceptor_.get()));
    }
}

void TcpServer::newConnection(int sockfd , const InetAddress& peerAddr)
{
    //轮询算法选择一个subLoop，用于管理对应的channel
    EventLoop* ioLoop = threadPool_->getNextLoop();
    char buf[64] = {0};
    ::snprintf(buf , sizeof buf , "-%s$%d",ipPort_.c_str() , nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;
    LOG_INFO("TcpServer::newConnection[%s]-new connection [%] from %s \n",
                name_.c_str() , connName.c_str() , peerAddr.toIp().c_str());
    
    //通过sockfd获取其绑定的本机ip地址和端口信息
    sockaddr_in local;
    ::bzero(&local , sizeof local);
    socklen_t addrlen = sizeof local;
    if(::getsockname(sockfd , (struct sockaddr*)&local , &addrlen) < 0)
    {
        LOG_ERROR("sockets::getLocalAddr");  
    }
    InetAddress localAddr(local);
    //根据连接成功的sockfd，创建TcpConnection连接对象
    TcpConnectionPtr conn(new TcpConnection(
            ioLoop,
            connName,
            sockfd,
            localAddr,
            peerAddr)
            );
    //TcpConnectionPtr conn = std::make_shared<TcpConnection>(ioLoop , connName , sockfd , localAddr , peerAddr);
    connections_[connName] = conn;
    //下面的回到都是用户设置给TcpServer => TcpConnection => Channel => Poller notify channel
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);

    //设置如何关闭连接的回调
    conn->setCloseCallback(
        std::bind(&TcpServer::removeConnection , this , std::placeholders::_1)
    );
    //直接调用TcpConnection::connectEstablished
    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished,conn));
}


void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop , this , conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
    LOG_INFO("TcoServer::removeConnectionInLoop[%s]-commection %s \n",name_,conn->name());
    connections_.erase(conn->name());
    EventLoop* ioLoop = conn->getLoop();
    ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed , conn));

}


