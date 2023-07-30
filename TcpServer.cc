#include "TcpServer.h"
#include "Logger.h"
#include <functional>


EventLoop* CheckLoopNotNull(EventLoop* loop)
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

void TcpServer::newConnection(int sockfd , const InetAddress& peerAddr)
{
    EventLoop* loop = threadPool_->getNextLoop();
    char buf[64];
    snprintf(buf,sizeof buf , "-%s:#%d",ipPort_.c_str(),nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;
    LOG_INFO("TcpServer::newConnection[%s] - new connection [%s] from %s \n",name_.c_str(),connName.c_str(),peerAddr.toIpPort().c_str());

}

void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{

}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{

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
