#include "../include/EventLoopThread.h"
#include "../include/EventLoop.h"

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb, const std::string &name):
    callback_(std::move(cb)),
    name_(name),
    exiting_(false),
    loop_(nullptr),
    thread_(std::bind(&EventLoopThread::threadFunc, this)),
    mutex_(),
    cond_()
{

}
EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if(!loop_)
    {
        loop_->quit();
        thread_.join();
    }
    
}

EventLoop* EventLoopThread::startLoop()
{
    thread_.start();  // 启动底层的新线程
    /*
        thread_在初始化的时候接受了EventLoopThread::threadFunc作为线程函数
        thread_.start()就会去执行threadFunc
        threadFunc中包含了EventLoop loop_的创建
        由于线程执行的先后顺序是不确定的，因此下面要返回loop
        必须等待threadFunc中创建好loop_
    */

    EventLoop *loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while(loop_ == nullptr)
        {
            cond_.wait(lock);
        }
        loop = loop_;
    }
    return loop;
}

// 下面这个方法，是在单独的新线程里面执行的
/*
    这个方法线程函数是给Thread thread_使用的
*/
void EventLoopThread::threadFunc()
{
    EventLoop loop;  // 创建一个独立的eventloop, 和上面的线程是一一对应，one loop per thread

    if(callback_)
    {   // 可以针对这个loop做一些事情，不是必要的
        callback_(&loop);
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }

    loop.loop();
    /*
        调用loop成员变量poller_->poll(), 相当于epoll_wait，开始监听
    */
    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
} 