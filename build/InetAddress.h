#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>

//封装socket地址类型

class InetAddress
{
public:
    explicit InetAddress(int port , std::string ip = "127.0.0.1");
    explicit InetAddress(const sockaddr_in& addr)
        :addr_(addr) {}
    
    std::string toIp() const;
    std::string toIpPort() const;  
    uint16_t toPort() const;

    const sockaddr * getSockAddr() const { return (sockaddr*)&addr_; }
private:
    sockaddr_in addr_;
};