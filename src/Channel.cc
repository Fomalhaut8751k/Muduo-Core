#include "../include/Channel.h"
#include "../include/Logger.h"
#include "../include/Alogger.h"
#include "../include/EventLoop.h"

#include "sys/epoll.h"


const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;  // 0x001 0x002
const int Channel::kWriteEvent = EPOLLOUT;   // 0x004

Channel::Channel(EventLoop* loop, int fd):
    loop_(loop), 
    fd_(fd),
    events_(0),
    revents_(0),
    index_(-1),
    tied_(false)
{
    
}

Channel::~Channel()
{

}

void Channel::tie(const std::shared_ptr<void>& obj)
{
    tie_ = obj;  // tie_是一个弱智能指针，用来观察强智能指针
    tied_ = true;  // 绑定了就置为true
}

// 当改变Channel所表示fd的events事件后，update负责在poller里面更改fd相应的事件epoll_fd
// Eventloop包含ChannelList   Poller， Channel需要通过EventLoop进而访问Poller
// 因为epoll相关的都被封装在了Poller
void Channel::update()
{   
    // 通过Channel所属的EventLoop，调用poller
    loop_->updateChannel(this);   
}

// 在Channel所属的Eventloop中
void Channel::remove()
{
    loop_->removeChannel(this);
}

void Channel::handleEvent(TimeStamp receiveTime)
{
    if(tied_)
    {   // 如果监控的对象还存在
        std::shared_ptr<void> guard = tie_.lock();
        if(guard)
        {
            handleEventWithGuard(receiveTime);
        }
    }
    else
    {
        handleEventWithGuard(receiveTime);
    }
}

// 根据具体接收到的事件，执行响应的回调函数
void Channel::handleEventWithGuard(TimeStamp receiveTime)
{
    char logbuf[1024] = {0};
    snprintf(logbuf, 1024, "channel handleEvent revents:%d\n", revents_);
    logger_->INFO(logbuf);
    // LOG_INFO("channel handleEvent revents:%d\n", revents_);
    
    // 接收到的事件：revents_
    if((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
    {
        if(closeCallback_) { closeCallback_(); }
    }
    if(revents_ & EPOLLERR)
    {
        if(errorCallback_) { errorCallback_(); }
    }
    if(revents_ & EPOLLIN)
    {
        if(readCallback_) { readCallback_(receiveTime); }
    }
    if(revents_ & EPOLLOUT)
    {
        if(writeCallback_) { writeCallback_(); }
    }
}

// 设置读回调函数对象
void Channel::setReadCallback(ReadEventCallback cb)
{
    readCallback_ = std::move(cb);
}   

// 设置写回调函数对象
void Channel::setWriteCallback(EventCallback cb)
{
    writeCallback_ = std::move(cb);
}

// 设置关闭回调函数
void Channel::setCloseCallback(EventCallback cb)
{
    closeCallback_ = std::move(cb);
}

// 设置错误回调函数对象
void Channel::setErrorCallback(EventCallback cb)
{
    errorCallback_ = std::move(cb);
}

int Channel::fd() const { return fd_; }
int Channel::events() const { return events_; }
void Channel::set_revents(int revt){ revents_ = revt; }
int Channel::revents() const { return revents_; }

// 设置fd对应的事件状态
void Channel::enableReading() { events_ |= kReadEvent; update(); }  
void Channel::enableWriting() { events_ |= kWriteEvent; update(); }
void Channel::disableReading() { events_ &= ~kReadEvent; update(); }
void Channel::disableWriting() { events_ &= ~kWriteEvent; update(); }
void Channel::disableAll() { events_ = kNoneEvent; update(); }

// 返回fd当前的事件状态
bool Channel::isNoneEvent() const { return events_ == kNoneEvent; }
bool Channel::isWriting() const { return events_ & kWriteEvent; }
bool Channel::isReading() const { return events_ & kReadEvent; }

int Channel::index() { return index_; }
void Channel::set_index(int idx) { index_ = idx; }

EventLoop* Channel::ownerLoop() { return loop_; }


