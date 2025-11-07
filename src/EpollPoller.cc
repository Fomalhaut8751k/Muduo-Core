#include "../include/EpollPoller.h"
#include "unistd.h"

#include <cassert>
#include <string.h>
#include <errno.h>

using namespace mymuduo::net;
using namespace mymuduo;

const int kNew = -1;  // channel未添加到poller中
const int kAdded = 1;  // channel已添加到poller中
const int kDeleted = 2;   // channel已添加到poller中但后续被删除


EpollPoller::EpollPoller(EventLoop* loop):
    Poller(loop),
    epollfd_(epoll_create1(EPOLL_CLOEXEC)), 
    events_(KInitEventListSize)
{
    if(epollfd_ < 0)
    {
        // 打印FATAL级别的日志
    }
}

EpollPoller::~EpollPoller()
{
    ::close(epollfd_);
}

TimeStamp EpollPoller::poll(int timeoutMs, ChannelList* activeChannels)
{
    /* 一条日志: "fd total count" << channels_.size() 【debug】*/
    int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs);
    int saveErrno = errno;
    TimeStamp now(TimeStamp::now());

    if(numEvents > 0)
    {
        /* 一条日志 %d events happened \n  */
        fillActiveChannels(numEvents, activeChannels);
        if(numEvents == events_.size())
        {   // 扩容
            events_.resize(events_.size() * 2);
        }
    }
    else if (numEvents == 0)  // 超时
    {
        /* 一条日志 %s timeout! \n, __FUNCTION__*/
    }
    else
    {
        if(saveErrno != EINTR)  // 不是外部中断
        {
            errno = saveErrno;
            /* 一条日志 EpollPoller::poll() err! */
        }
    }

    return now;
}

void EpollPoller::updateChannel(Channel* channel)
{
    // hasChannel(channel)是判断是否在EventLoop中，而非在Poller中
    const int index = channel->index();  // 用这个来判断是否在Poller中
    
    /* 一条日志信息, fd, events, index */

    int fd = channel->fd();
    if(index == kNew || index == kDeleted)  // 如果是新的，或者已删除了的
    {   // epoll_ctl(  , EPOLL_CTL_ADD,  )
        if(index == kNew)
        {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        }
        if(index == kDeleted)
        {   // channels_还存在，但是并没有被注册到Poller中就是删除？
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
        }
        // epoll_ctl(epollfd_, EPOLL_CTL_ADD, channel->fd(), &event);
        update(EPOLL_CTL_ADD, channel);
        // 修改channel的状态
        channel->set_index(kAdded);
    }
    else  // 已经存在的
    {   // epoll_ctl( , EPOLL_CTL_MOD,  )
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] == channel);
        assert(channel->index() == kAdded);
        if(channel->isNoneEvent())  // 如果对任何事件都不管兴趣，就不需要监听了
        {   // 就删除掉
            // epoll_ctl(epollfd_, EPOLL_CTL_DEL, channel->fd(), &event);
            update(EPOLL_CTL_DEL, channel);
            // 修改channel的状态
            channel->set_index(kDeleted);
        }
        else
        {
            // epoll_ctl(epollfd_, EPOLL_CTL_MOD, channel->fd(), &event);
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EpollPoller::removeChannel(Channel* channel)
{
    /*
        在channels_中的channel状态可能是kAdded或者kDeleted，而不会是kNew
        调用removeChannel()后就会把对应的channel从channels_删除掉，然后状态置为kNew
    */

    int fd = channel->fd();
    // 只有在Poller中，并且没有感兴趣的事件时才可以删除掉
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->isNoneEvent()); 

    const int index = channel->index();

    assert(index == kAdded || index == kDeleted);  // 这两个状态下

    size_t n = channels_.erase(fd);  // 从channels_中删除 
    
    if(index == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    // 修改channel的状态
    channel->set_index(kNew);
}


// 更新Channel通道，epoll_ctl() add/mod/del
void EpollPoller::update(int operation, Channel* channel)
{
    struct epoll_event event;
    memset(&event, 0, sizeof(event));
    // event.data.fd = channel->fd();
    event.data.ptr = channel;  // 可以携带channel的各种参数包括fd,因为是union所以只能使用一个
    event.events = channel->events();

    if(::epoll_ctl(epollfd_, operation, channel->fd(), &event) < 0)
    {   // 如果出错了，先看看是哪个operation
        if(operation == EPOLL_CTL_DEL)
        {
            /* 一条日志信息，删除失败ERROR */
        }
        else
        {
            /* 一条日志信息，添加或修改失败FATAL */
        }
    }
}


// 填写活跃的连接
void EpollPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const
{
    for(int i = 0; i < numEvents; ++i)
    {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        int fd = channel->fd();
        ChannelMap::const_iterator it = channels_.find(fd);
        
        assert(it != channels_.end());
        assert(it->second == channel);

        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }
    // EventLoop就拿到了它的poller给它返回的所有事件的channel列表了
}