#include "../include/Poller.h"
#include "../include/EpollPoller.h"
// #include "PollPoller.h"

#include <stdlib.h>


Poller* Poller::newDefaultPoller(EventLoop* loop)
{
    if(::getenv("MUDUO_USE_POLL"))
    {
        // return new PollPoller();
        return nullptr;
    }
    else
    {
        return new EpollPoller(loop);
        // return nullptr;
        // return std::make_shared<EpollPoller>(loop);
    }
}