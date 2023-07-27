#pragma once

#include "nocopyable.h"
#include "Socket.h"
#include "Channel.h"
#include "InetAddress.h"
#include <functional>

class EventLoop;
class InetAddress;

class Acceptor : nocopyable
{
public:
    using NewConnectionCallback = std::function<void(int , const InetAddress&)>;

    Acceptor(EventLoop* loop , const InetAddress & listenAddr , bool reuseport);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback& cb) { newConnectCallback_ = cb; }
    bool listenning() { return listenning_; }

    void listen();
private:

    void handleRead();

    EventLoop* loop_;
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectCallback_;
    bool listenning_;
    int idleFd_;
};