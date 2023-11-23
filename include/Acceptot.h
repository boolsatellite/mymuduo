//
// Created by satellite on 2023-11-23.
//

#ifndef MYMUDUO_ACCEPTOT_H
#define MYMUDUO_ACCEPTOT_H

#include <utility>

#include "nocopyable.h"
#include "Socket.h"
#include "Channel.h"

class EventLoop;
class InetAddress;

class Acceptor : nocopyable {
public:
    using NewConnectionCallback = std::function<void(int , const InetAddress&)>;

    Acceptor(EventLoop* loop , const InetAddress& listenAddr , bool reuseport);
    ~Acceptor();

    void setNewConnectionCallback(NewConnectionCallback cb) {
        newConnectionCallback_ = std::move(cb);
    }

    void listen();

    bool listening() const { return listenning_; }


private:
    void handleRead();

    EventLoop* loop_;
    Socket acceptSocket_;           //AcceptSocket是非阻塞的sockfd
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listenning_;

};


#endif //MYMUDUO_ACCEPTOT_H
