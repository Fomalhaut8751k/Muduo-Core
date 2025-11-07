#ifndef EPOLLPOLLER_H
#define EPOLLPOLLER_H

#include "Poller.h"

#include <vector>
#include <sys/epoll.h>

using namespace mymuduo::net;

namespace mymuduo
{
namespace net
{
    class EpollPoller: public Poller
    {
    public:
        EpollPoller(EventLoop* loop);

        ~EpollPoller() override;

        // 重写基类的抽象方法
        virtual TimeStamp poll(int timeoutMs, ChannelList* activeChannels) override;
        virtual void updateChannel(Channel* channel) override;
        virtual void removeChannel(Channel* channel) override;

    private:
        static const int KInitEventListSize = 16;

        // 填写活跃的连接
        void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
        // 更新channel的通道
        void update(int operation, Channel* channel);

        using EventList = std::vector<epoll_event>;

        int epollfd_;
        EventList events_;  // 应该是给epoll_wait用的
    };
}

}


#endif