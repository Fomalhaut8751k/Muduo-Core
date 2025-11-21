#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include <functional>

#include "Socket.h"
#include "Channel.h"
#include "noncopyable.h"

class EventLoop;
class InetAddress;

class Acceptor: noncopyable
{
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;

    Acceptor(EventLoop* loop, const InetAddress& listenaddr, bool reuseport);
    ~Acceptor();

    void setNewConnectionCallback(const NewConnectionCallback& cb);
    bool listening() const;
    void listen();

private:
    void handleRead();

    EventLoop* loop_;   // Acceptor用的就是用户定义的那个baseloop，也就是mainloop
    Socket acceptSocket_;
    Channel acceptChannel_;

    NewConnectionCallback newConnectionCallback_;
    bool listening_;
};

#endif