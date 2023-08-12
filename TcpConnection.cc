#include "TcpConnection.h"
#include "Logger.h"
#include "Socket.h"
#include "EventLoop.h"
#include "Channel.h"
#include "Callbacks.h"
#include <functional>
#include <bits/shared_ptr.h>
#include <errno.h>
#include <unistd.h>
#include <string>

static EventLoop* CheckLoopNotNull(EventLoop* loop)
{
    if(loop == nullptr)
    {
        LOG_FATAL("%s:%s:%d subLoop is null \n",__FILE__,__FUNCTION__,__LINE__);
    }
    return loop;
}

TcpConnection::TcpConnection(EventLoop *loop, const std::string &nameArg
            ,int sockfd, const InetAddress &localAddr, const InetAddress &peerAddr)
            :loop_(CheckLoopNotNull(loop))
            ,name_(nameArg)
            ,state_(kConnecting)
            ,reading_(true)
            ,socket_(new Socket(sockfd))        //TcpServer传递sockfd
            ,channel_(new Channel(loop , sockfd))
            ,localAddr_(localAddr)
            ,peerAddr_(peerAddr)
            ,highWaterMark_(64 * 1024 * 1024)   //64M
            {
                //给channel设置相应的回调函数，poller回给channel通知感兴趣的事件发生，channel回调对应的事件
                  channel_->setReadCallback(
                      std::bind(&TcpConnection::handleRead , this , std::placeholders::_1)
                  );
                  channel_->setWriteCallback(
                      std::bind(&TcpConnection::handleWrite,this)
                  );
                  channel_->setCloseCallback(
                      std::bind(&TcpConnection::handleClose,this)
                  );
                  channel_->setErrorCallback(
                      std::bind(&TcpConnection::handleError,this)
                  );
                  LOG_INFO("TcpConnection::ctor[%s] at fd=%d \n",name_.c_str(),sockfd);
                  socket_->setKeepAlive(true);
            }
TcpConnection::~TcpConnection() 
{
    LOG_INFO("TcpConnection::dtor[%s] at fd=%d state=%d \n",
                name_.c_str() , channel_->fd() , (int)state_);
}

void TcpConnection::handleRead(TimeStamp receiveTime)
{
    int saveErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd() , &saveErrno);
    if(n > 0)
    {
        //已连接用户，有可读事件发生了，调用用户传入的回调onMessage
       TcpConnection::messageCallback_(shared_from_this() , &inputBuffer_ , receiveTime);
    }
    else if(n == 0)
    {
        handleClose();
    }
    else
    {
        errno = saveErrno;
        LOG_ERROR("TcpConnection::handleRead\n");
        handleError();
    }
}

void TcpConnection::handleWrite()
{
    int saveErrno = 0;
    ssize_t n = outputBuffer_.writeFd(channel_->fd() , &saveErrno);
    if(n > 0)
    {
        outputBuffer_.retrieve(n);
        if(outputBuffer_.readableBytes() == 0)
        {
            channel_->disableWriting();
            if(writeCompleteCallback_)
            {
                loop_->queueInLoop(
                    std::bind(&TcpConnection::writeCompleteCallback_,shared_from_this())
                );
            }
            if(state_ == kDisconnecting)
            {
                shutdownInLoop();
            }
        }
        else
        {
            LOG_ERROR("TcpConnection::handleWrite");
        }
    }
    else
    {
        LOG_ERROR("TcpConnection fd=%d is down , no more writing \n",channel_->fd());
    }
}

void TcpConnection::handleClose()
{
    LOG_INFO("fd=%d state=%d \n",channel_->fd() , (int)state_);  //(int)state_防止atomoc_int发生拷：不知道原因
    setState(kDisconnected);
    channel_->disableAll();
    TcpConnectionPtr connptr{shared_from_this()};
    connetionCallback_(connptr);
    closeCallback_(connptr);
}

void TcpConnection::handleError()
{
    int optval;
    int err = 0;
    socklen_t optlen = static_cast<socklen_t>(sizeof(optval));

    if(::getsockopt(channel_->fd(), SOL_SOCKET , SO_ERROR , &optval , &optlen) < 0)
    {
        err = errno;
    }
    else
    {
        err = optval; 
    }
    LOG_ERROR("TcpConnnetion::handError[%s] SO_ERROR %d",name_.c_str(),err)
}


void TcpConnection::sendInLoop(const void* data, size_t len)
{
//发送数据应用写的快，内核发送慢，需要将数据写入缓冲区，
//而且设置了水位线防止发送太快
    size_t nwrote = 0;
    ssize_t remaining = len;  //未发送完的数据
    bool faultError = false;

    if(state_ = kDisconnected)  //之前调用conntion的shutdown函数不能继续发送
    {
        LOG_ERROR("disconnected , give up writing~ \n");
        return;
    }
    //表示channel第一次开始写数据，而且缓冲区没有待发送数据
    if(!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
    {
        nwrote = ::write(channel_->fd() , data , len);
        if(nwrote >= 0)
        {
            remaining = len - nwrote;
            if(remaining == 0 && writeCompleteCallback_)
            {
                //既然在这里就将数据全部发送完成了，就不用给channel设置epollout事件了
                loop_->queueInLoop(bind(writeCompleteCallback_,shared_from_this()));
            }
        }
        else //nwrote < 0
        {
            nwrote = 0;
            if(errno != EWOULDBLOCK)
            {
                LOG_ERROR("TcpConnection::sendInLoop \n");
                if(errno == EPIPE || errno == ECONNRESET)
                {
                    faultError = true;
                }
            }
        }
    }
    //当前write并没有将数据全部发送出去，剩余数据需要保存到缓冲区中
    //然后给poller注册epollout事件，poller发现tcp发送缓冲区中有空间，会通知相应的sock-channel
    //调用writeCallback，回调函数及TCP Connection::handleWrite方法，将缓冲区的数据全部发出
    if(!faultError && remaining > 0)  
    {
        size_t oldlen = outputBuffer_.readableBytes();  //发送缓冲区剩余待发送数据的长度
        if(oldlen+remaining >= highWaterMark_ &&
            oldlen < highWaterMark_ &&
            highWaterCallback_)
            {
                loop_->queueInLoop(std::bind(TcpConnection::highWaterCallback_
                            , shared_from_this() , oldlen+remaining));
            }
        outputBuffer_.append((char*)data + nwrote , remaining);
        if(!channel_->isWriting())
        {
            channel_->enableWriting();  //channel注册写事件
        }
    }
}

void TcpConnection::send(const std::string& buf)
{
    if(state_ == kConnected)
    {
        if(loop_->isInLoopThread())
        {
            sendInLoop(buf.c_str() , buf.size());
        }
        else
        {
            loop_->runInLoop(bind(&TcpConnection::sendInLoop
                        ,this,buf.c_str(),buf.size()));
        }
    }
}


void TcpConnection::connectEstablished()
{
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading();    //向poller注册channel的epollin事件
    ConnectionCallback(shared_from_this());  //连接成功执行回调函数
}

void TcpConnection::connectDestroyed()
{
    if(state_ == kConnected)
    {
        setState(kDisconnected);
        channel_->disableAll();  //把channel的所有感兴趣事件从poller中delete
        ConnectionCallback(shared_from_this());
    }
    channel_->remove();
}


void TcpConnection::shutdown()
{
    if(state_ == kConnected)
    {
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop,this));
    }
}

void TcpConnection::shutdownInLoop()
{
    if(!channel_->isWriting())  //说明outputBuffer中数据以及全部发送完成
    {
        socket_->shutdownWrite();
    }
}

