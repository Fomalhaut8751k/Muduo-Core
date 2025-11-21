#include "../include/InetAddress.h"

#include <string.h> 

InetAddress::InetAddress(uint16_t port, std::string ip)
{
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = inet_addr(ip.c_str());
    addr_.sin_port = htons(port);
}

InetAddress::InetAddress(const struct sockaddr_in& addr)
    :addr_(addr)
{
    
}

// 返回ip地址
std::string InetAddress::toIp() const
{
    char buf[64] = {0};
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
    return buf;
}

// 返回ip地址加端口号
std::string InetAddress::toIpPort() const
{
    char buf[64] = {0};
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
    size_t end = strlen(buf);
    uint16_t port = ::ntohs(addr_.sin_port);
    sprintf(buf+end, ":%u", port);
    return buf;
}

// 返回端口号
uint16_t InetAddress::toPort() const
{
    return ::ntohs(addr_.sin_port);
}

const struct sockaddr_in* InetAddress::getSockAddr() const
{
    return &addr_;
}

void InetAddress::setSockAddr(const sockaddr_in& addr)
{
    addr_ = addr;
}


/* 测试 */
// #include <iostream>

// int main()
// {
//     InetAddress addr(8080);
//     std::cout << "ip addr: " << addr.toIp() << std::endl;
//     std::cout << "port: " << addr.toPort() << std::endl;
//     std::cout << "ip addr + port: " << addr.toIpPort() << std::endl;

//     return 0;
// }