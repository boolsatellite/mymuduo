//
// Created by satellite on 2023-11-09.
//

#include <cstring>
#include <arpa/inet.h>
#include "InetAddress.h"
#include "string.h"

InetAddress::InetAddress(unsigned short port, std::string ip) {
    bzero(&addr_ , sizeof (addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    //inet_pton(AF_INET , ip.c_str() , &addr_.sin_addr);
    addr_.sin_addr.s_addr = inet_addr(ip.c_str());
}

InetAddress::InetAddress(const sockaddr_in &addr) : addr_(addr) {}

std::string InetAddress::toIp() const {
    char buf[64] = {0};
    inet_ntop(AF_INET,&addr_.sin_addr,buf,sizeof(buf));
    return std::string (buf);
}

std::string InetAddress::toIpPort() const {
    char buf[64] = {0};
    inet_ntop(AF_INET,&addr_.sin_addr,buf,sizeof(buf));
    size_t end = strlen(buf);
    unsigned short port = ntohs(addr_.sin_port);
    sprintf(buf+end , ":%u" , port);
    return std::string(buf);
}

const sockaddr *InetAddress::getSockAddr() const {
    return (const sockaddr*)&addr_;
}




