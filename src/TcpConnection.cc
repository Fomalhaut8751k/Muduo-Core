#include "../include/TcpConnection.h"
#include "../include/Socket.h"
#include "../include/Channel.h"
#include "../include/EventLoop.h"

#include <functional>

static EventLoop* CheckLoopNotNull(EventLoop* loop)
{
    if(!loop) { /*一条日志 FATAL %s:%s:%s mainLoop is null！ \n, __FILE__, __FUNCTION__, __LINE__*/ }
    return loop;
}

TcpConnection::TcpConnection(EventLoop* loop, 
                             const std::string nameArg, 
                             int sockfd, 
                             const InetAddress& localAddr, 
                             const InetAddress& peeraddr):
    loop_(CheckLoopNotNull(loop)),
    name_(nameArg),
    socket_(new Socket(sockfd)),
    channel_(new Channel(loop_, sockfd)),
    localAddr_(localAddr),
    peerAddr_(peerAddr_),
    reading_(true),
    state_(kConnecting),
    highWaterMark_(64 * 1024 * 1024)
{
    channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, std::placeholders::_1));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));

    /*一条日志，INFO "TcpConnection::ctor[%s] at fd=%d\n", name_c_str(), sockfd*/
    
    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
    /* 一条日志 TcpConnection::dtor[%s] at fd=%d state=%d \n name.c_str(), channel_->fd(), state_;*/
}

EventLoop* TcpConnection::getLoop() const
{ 
    return loop_; 
}

const std::string TcpConnection::name() const
{
    return name_; 
}

const InetAddress& TcpConnection::localAddress() const
{
    return localAddr_;
}

const InetAddress& TcpConnection::peerAddress() const
{
    return peerAddr_;
}

bool TcpConnection::connected() const
{
    return state_ == kConnecting;
}

void TcpConnection::setConnectionCallback(const ConnectionCallback& cb)
{
    connectionCallback_ = cb;
}

void TcpConnection::setMessageCallback(const MessageCallback& cb)
{   
    messageCallback_ = cb;
}

void TcpConnection::setWriteCallback(const WriteCompleteCallback& cb)
{
    writeCompleteCallback_ = cb;
}

void TcpConnection::setCloseCallback(const CloseCallback& cb)
{
    closeCallback_ = cb;
}

void TcpConnection::setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark)
{
    highWaterMarkCallback_ = cb;
    highWaterMark_ = highWaterMark;
}