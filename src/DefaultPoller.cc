#include "../include/Poller.h"
// #include "EPollPoller.h"
// #include "PollPoller.h"

#include <stdlib.h>

// using namespace mymuduo;
using namespace mymuduo::net;

Poller* Poller::newDefaultPoller(EventLoop* loop)
{
    if(::getenv("MUDUO_USE_POLL"))
    {
        // return new PollPoller();
        return nullptr;
    }
    else
    {
        // return new EpollPoller();
        return nullptr;
    }
}