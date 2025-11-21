#ifndef INETADDRESS_H
#define INETADDRESS_H

#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>

#include "copyable.h"

class InetAddress: public copyable
{
public:
    explicit InetAddress(uint16_t port = 0, std::string ip = "127.0.0.1");
    explicit InetAddress(const struct sockaddr_in& addr);

    std::string toIp() const;
    std::string toIpPort() const;
    uint16_t toPort() const;
    const struct sockaddr_in* getSockAddr() const;

    void setSockAddr(const sockaddr_in& addr);

private:
    struct sockaddr_in addr_;
};

#endif