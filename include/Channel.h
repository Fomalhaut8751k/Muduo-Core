#ifndef CHANNEL_H
#define CHANNEL_H

#include "noncopyable.h"
#include "TimeStamp.h"

#include <functional>
#include <memory>


class EventLoop;
// class TimeStamp;

/*
Channel 理解为通道，封装了sockfd和其感兴趣的event，如EPOLLIN,EPOLLOUT事件
还绑定了poller返回的具体事件
*/

class Channel: noncopyable
{
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(TimeStamp)>;

private:
    /*
        应该是1,2,4，这样置位的时候，如果events_为3
        就表示fd对读事件(1)和写事件(3)感兴趣
    */
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop *loop_;  // 事件循环
    const int fd_;     // fd, Poller监听的对象
    int events_;       // 注册fd感兴趣的事件
    int revents_;      // Poller返回的具体发生的事件
    int index_;
    
    std::weak_ptr<void> tie_;
    bool tied_;

    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;

    void update();  // 包含epoll_ctl()

    void handleEventWithGuard(TimeStamp receiveTime);

public:
    Channel(EventLoop* loop, int fd);
    ~Channel();

    // fd得到Poller通知以后，处理事件的回调方法
    void handleEvent(TimeStamp receiveTime);

    // 设置读回调函数对象
    void setReadCallback(ReadEventCallback cb);

    // 设置写回调函数对象
    void setWriteCallback(EventCallback cb);

    // 设置关闭回调函数
    void setCloseCallback(EventCallback cb);

    // 设置错误回调函数对象
    void setErrorCallback(EventCallback cb);

    // 防止当channel被手动remove掉，channel还在执行回调操作
    void tie(const std::shared_ptr<void>& obj);

    int fd() const;

    int events() const;

    int set_revents(int revt);   // 对外提供的设置revents_的接口函数

    int revents() const;

    // 设置fd对应的事件状态
    void enableReading();  // 如果fd对读事件感兴趣，就把events_设置上读事件的标志
    void enableWriting();  // 如果fd对写事件感兴趣，就把events_设置上写事件的标志
    void disableReading();  // 如果不感兴趣，就把events_对应读事件的位置置0
    void disableWriting();  // 如果不感兴趣，就把events_对应写事件的位置置0
    void disableAll();     // 对所有事件都不感兴趣

    // 返回fd当前的事件状态
    bool isNoneEvent() const; 
    bool isWriting() const;
    bool isReading() const;

    int index();
    void set_index(int idx);

    // 当前Channel所属的eventloop
    EventLoop* ownerLoop();
    void remove();

};


#endif