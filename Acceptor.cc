#include "Acceptor.h"
#include "Logger.h"
#include "Channel.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

static int createNoblocking()
{
    int sockfd = socket(AF_INET,SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC , 0);
    if(sockfd < 0)
    {
        LOG_FATAL("%s:%s:%d listen socket create err:%d \n",__FILE__,__FUNCTION__,__LINE__,errno); 
    }
    return sockfd;
}


Acceptor::Acceptor(EventLoop* loop , const InetAddress & listenAddr , bool reuseport)
    : loop_(loop)
    , acceptSocket_(::createNoblocking())
    , acceptChannel_(loop,acceptSocket_.fd())
    , listenning_(false)
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(reuseport);
    acceptSocket_.bindAddress(listenAddr);
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead,this));
}

Acceptor::~Acceptor()
{
    acceptChannel_.disableAll();
    acceptChannel_.remove();
}

void Acceptor::listen()
{
    listenning_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}

void Acceptor::handleRead()
{
    InetAddress peerAddr;
    int connfd = acceptSocket_.accept(&peerAddr);
    if(connfd >=0 )
    {
        if(newConnectCallback_)
        {
            newConnectCallback_(connfd,peerAddr);
        }
        else
        {
            ::close(connfd);
        }
    }
    else
    {
        LOG_ERROR("%s:%s:%d accept error:%d \n", __FILE__ , __FUNCTION__ , __LINE__ , errno);
        if(errno == EMFILE)
        {
            LOG_ERROR("%s:%s:%d sockfd reacked limit \n",__FILE__ , __FUNCTION__ , __LINE__);
        }
    }
}
