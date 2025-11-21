#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include "noncopyable.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include "Buffer.h"
#include "TimeStamp.h"

#include <memory>
#include <atomic>
#include <string>

class Channel;
class EventLoop;
class Socket;

/*
 TcpServer => Acceptor => 有一个新用户连接，通过accept函数拿到connfd
 => TcpConnection 设置回调 => Channel => Poller => Channel的回调操作
*/

class TcpConnection: noncopyable, public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop* loop, 
                  const std::string nameArg, 
                  int sockfd, 
                  const InetAddress& localAddr, 
                  const InetAddress& peeraddr);

    ~TcpConnection();

    EventLoop* getLoop() const;
    const std::string name() const;
    const InetAddress& localAddress() const;
    const InetAddress& peerAddress() const;

    bool connected() const;

    // 发送数据
    void send(const void* message, int len);
    // 关闭连接
    void shutdown();

    // 设置回调函数
    void setConnectionCallback(const ConnectionCallback& cb);
    void setMessageCallback(const MessageCallback& cb);
    void setWriteCallback(const WriteCompleteCallback& cb);
    void setCloseCallback(const CloseCallback& cb);
    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark);

    // 连接建立和连接销毁
    void connectEstablished();
    void connectDestroyed();

private:
    enum StateE { kDisconnected, kConnecting, kConnected, kDisconnected };
    
    void handleRead(TimeStamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const void* message, size_t len);
    void shutdownInLoop();
    
    EventLoop *loop_;  // 这里绝对不是baseloop_, 因为TcpConnection都是在subloop管理的
    const std::string name_;
    std::atomic_int state_;
    bool reading_;

    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    const InetAddress localAddr_;
    const InetAddress peerAddr_;

    ConnectionCallback connectionCallback_;  // 有新连接时的回调
    MessageCallback messageCallback_;   // 有读写消息时的回调
    WriteCompleteCallback writeCompleteCallback_;  // 消息发送完成以后的回调
    CloseCallback closeCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;

    size_t highWaterMark_;
    Buffer inputBuffer_;
    Buffer outputBuffer_;
};

#endif