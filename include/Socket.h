#ifndef SOCKET_H
#define SOCKET_H

#include "noncopyable.h"

class InetAddress;

// 封装socket fd
class Socket: noncopyable
{
public:
    explicit Socket(int sockfd);
    ~Socket();

    int fd() const;
    void bindAddress(const InetAddress& Localaddr);
    void listen();
    int accept(InetAddress* peeraddr);

    void shutdownWrite();

    void setTcpNoDelay(bool on);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);

private:
    const int sockfd_;
};

#endif