#include "../include/Socket.h"
#include "../include/InetAddress.h"
#include "../include/Logger.h"

#include "unistd.h"
#include "string.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>

Socket::Socket(int sockfd):
    sockfd_(sockfd)
{}

Socket::~Socket()
{
    ::close(sockfd_);
}

int Socket::fd() const
{
    return sockfd_;
}

void Socket::bindAddress(const InetAddress& Localaddr)
{
    if(0 !=::bind(sockfd_, (sockaddr*)Localaddr.getSockAddr(), sizeof(sockaddr_in)))
    {
        LOG_FATAL("bind sockfd:%d fail \n", sockfd_);
    }   
}

void Socket::listen()
{
    if(0 != ::listen(sockfd_, 1024))
    {
        LOG_FATAL("listen sockfd:%d fail \n", sockfd_);
    }
}

int Socket::accept(InetAddress* peeraddr)
{
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t addr_len = sizeof(addr);
    int connfd = ::accept4(sockfd_, (sockaddr*)&addr, &addr_len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if(0 <= connfd)
    {
        peeraddr->setSockAddr(addr);
    }
    return connfd;
}

void Socket::shutdownWrite()
{
    if(0 > ::shutdown(sockfd_, SHUT_WR))
    {
        LOG_ERROR("shutdownWrite error");
    }
}

void Socket::setTcpNoDelay(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
}

void Socket::setReuseAddr(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}

void Socket::setReusePort(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
}

void Socket::setKeepAlive(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
}