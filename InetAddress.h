#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string>

//封装socket地址类型

class InetAddress
{
public:
    explicit InetAddress(int port = 0 , std::string ip = "127.0.0.1");
    explicit InetAddress(const sockaddr_in& addr)
        :addr_(addr) {}
    
    std::string toIp() const;
    std::string toIpPort() const;  
    uint16_t toPort() const;

    const sockaddr * getSockAddr() const { return (sockaddr*)&addr_; }
    void setSockAddrInet(struct sockaddr_in& addr) { addr_ = addr; }
private:
    sockaddr_in addr_;
};