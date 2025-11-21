#ifndef EVENTLOOPTHREADPOOL_H
#define EVENTLOOPTHREADPOOL_H

#include "noncopyable.h"

#include <functional>
#include <string>
#include <vector>
#include <memory>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool: noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThreadPool(EventLoop* baseloop, const std::string& nameArg);
    ~EventLoopThreadPool();

    void setThreadNum(int numThreads);

    void start(const ThreadInitCallback& cb = ThreadInitCallback());

    // 如果工作在多线程中，baseloop_,默认以轮询的方式分配channel给subloop
    EventLoop* getNextLoop();

    std::vector<EventLoop*> getAllLoops();  // 返回池里所有的loop

    bool started() const;
    const std::string name() const;

private:
    EventLoop* baseLoop_;  
    std::string name_;
    bool started_;
    int numThread_;
    int next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;
};

#endif