#include "../include/EventLoopThreadPool.h"
#include "../include/EventLoopThread.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseloop, const std::string& nameArg):
    baseLoop_(baseloop),
    name_(nameArg),
    started_(false),
    numThread_(0),
    next_(0)
{

}

EventLoopThreadPool::~EventLoopThreadPool()
{}

void EventLoopThreadPool::setThreadNum(int numThreads){ numThread_ = numThreads; }

void EventLoopThreadPool::start(const ThreadInitCallback& cb)
{
    started_ = true;
    for(int i = 0; i < numThread_; ++i)
    {
        char buf[name_.size() + 32];
        snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
        EventLoopThread* t = new EventLoopThread(cb, buf);
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        loops_.push_back(t->startLoop());
    }

    if(numThread_ == 0 && cb)
    {
        cb(baseLoop_);
    }

}

bool EventLoopThreadPool::started() const { return started_; }

const std::string EventLoopThreadPool::name() const { return name_; }

// 如果工作在多线程中，baseloop_默认以轮询的方式分配channel给subloop
EventLoop* EventLoopThreadPool::getNextLoop()
{
    EventLoop* loop = baseLoop_;
    if(!loops_.empty())
    {
        loop = loops_[next_];
        ++next_;
        if(next_ >= loops_.size()) { next_ = 0; }
    }
    return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::getAllLoops()
{
    if(loops_.empty())
    {
        return std::vector<EventLoop*>(1, baseLoop_);
    }
    return loops_;
}