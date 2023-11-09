//
// Created by satellite on 2023-11-09.
//

#ifndef MYMUDUO_INETADDRESS_H
#define MYMUDUO_INETADDRESS_H


#include <netinet/in.h>
#include <string>


class InetAddress {
public:
    explicit InetAddress(unsigned short port , std::string ip = "127.0.0.1");
    explicit InetAddress(const sockaddr_in& addr);
    std::string toIp() const;
    std::string toIpPort() const ;
    const sockaddr* getSockAddr() const ;

private:
    sockaddr_in addr_;
};


#endif //MYMUDUO_INETADDRESS_H
