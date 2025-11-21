#ifndef POLLER_H
#define POLLER_H

#include "noncopyable.h"
#include "TimeStamp.h"
#include "Channel.h"

#include <vector>
#include <unordered_map>

// 如果需要使用的是变量的指针而非变量本身，就不需要包含对应头文件(对象长度不固定，但指针固定)
// class Channel;
class EventLoop;  

// muduo库中多路事件分发器的核心，IO复用模块
class Poller: noncopyable
{
public:
    using ChannelList = std::vector<Channel*>;

    Poller(EventLoop* loop);

    virtual ~Poller() = default;

    // 给所有IO复用保留统一的接口
    virtual TimeStamp poll(int timeoutMs, ChannelList* activeChannels) = 0;
    virtual void updateChannel(Channel* channel) = 0;
    virtual void removeChannel(Channel* channel) = 0;

    // 判断参数channel是否在当前的Poller中
    bool hasChannel(Channel* channel) const;
    
    // EventLoop可以通过该接口获取默认的IO复用的具体实现
    static Poller* newDefaultPoller(EventLoop* loop);

protected:
    // map的key: sockfd  value: sockfd所属的channel通道类型
    using ChannelMap = std::unordered_map<int, Channel*>;
    
    ChannelMap channels_;

private:
    EventLoop* ownerLoop_;
    
};



#endif