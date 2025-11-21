#include "../include/TcpServer.h"

#include <functional>

EventLoop* CheckLoopNotNull(EventLoop* loop)
{
    if(!loop) { /*一条日志 FATAL %s:%s:%s mainLoop is null！ \n, __FILE__, __FUNCTION__, __LINE__*/ }
    return loop;
}

TcpServer::TcpServer(EventLoop* loop, const InetAddress& addr, const std::string& nameArg, Option option):
    loop_(CheckLoopNotNull(loop)),
    ipPort_(addr.toIpPort()),
    name_(nameArg),
    acceptor_(new Acceptor(loop, addr, option==kReusePort)),
    threadpool_(new EventLoopThreadPool(loop, name_)),
    connectionCallback_(),
    messageCallback_(),
    nextConnId_()
{
    // 当有新用户连接时，会执行TcpServer::newConnection回调
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, 
                                        std::placeholders::_1, std::placeholders::_2));
    
}

// 设置底层subloop个数
void TcpServer::setThreadNum(int numThreads)
{
    threadpool_->setThreadNum(numThreads);
}

// 开启服务器监听
void TcpServer::start()
{
    if(started_++ == 0)  // 防止一个TcpServer对象被多次start
    {
        threadpool_->start(threadInitCallback_);
        // 在当前loop中执行cb
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));  
    }
    
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr)
{

}


void TcpServer::setThreadInitCallback(const ThreadInitCallback& cb){ threadInitCallback_ = cb; }

void TcpServer::setConnectionCallback(const ConnectionCallback& cb){ connectionCallback_ = cb; }

void TcpServer::setMessageCallback(const MessageCallback& cb){ messageCallback_ = cb; }

void TcpServer::setWriteCompleteCallback(const WriteCompleteCallback& cb){ writeCompleteCallback_ = cb; }