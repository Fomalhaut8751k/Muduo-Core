#include "../include/Acceptor.h"
#include "../include/Alogger.h"
#include "../include/Logger.h"
#include "../include/InetAddress.h"

#include <unistd.h>
#include <errno.h>

static int createNonblocking()
{
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if(sockfd < 0)
    {
        char logbuf[1024] = {0};
        snprintf(logbuf, 1024, "%s:%s:%d listen socket create err:%d \n", __FILE__, __FUNCTION__, __LINE__, errno);
        logger_->FATAL(logbuf);
        // LOG_FATAL("%s:%s:%d listen socket create err:%d \n", __FILE__, __FUNCTION__, __LINE__, errno);
    }

    return sockfd;
}

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenaddr, bool reuseport):
    loop_(loop),
    acceptSocket_(createNonblocking()),
    listening_(false),
    acceptChannel_(loop, acceptSocket_.fd())
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(true);
    acceptSocket_.bindAddress(listenaddr);
    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor()
{
    acceptChannel_.disableAll();
    acceptChannel_.remove();
}

void Acceptor::setNewConnectionCallback(const NewConnectionCallback& cb)
{
    newConnectionCallback_ = std::move(cb);
}

bool Acceptor::listening() const
{
    return listening_;
}

void Acceptor::listen()
{
    listening_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}

// listenfd有事件发生了，就是有新用户连接了
void Acceptor::handleRead()
{
    InetAddress peerAddr;
    int connfd = acceptSocket_.accept(&peerAddr);
    if(connfd > 0)
    {
        if(newConnectionCallback_)
        {
            newConnectionCallback_(connfd, peerAddr);
        }
        else
        {
            ::close(connfd);
        }
    }
    else
    {   
        {
            char logbuf[1024] = {0};
            snprintf(logbuf, 1024, "%s:%s:%d accept err:%d \n", __FILE__, __FUNCTION__, __LINE__, errno);
            logger_->ERROR(logbuf);
        }
        // LOG_ERROR("%s:%s:%d accept err:%d \n", __FILE__, __FUNCTION__, __LINE__, errno);
        
        if(errno == EMFILE)
        {
            char logbuf[1024] = {0};
            snprintf(logbuf, 1024, "%s:%s:%d sockfd reached limit! \n", __FILE__, __FUNCTION__, __LINE__);
            logger_->ERROR(logbuf);
            // LOG_ERROR("%s:%s:%d sockfd reached limit! \n", __FILE__, __FUNCTION__, __LINE__);
        }
    }
}