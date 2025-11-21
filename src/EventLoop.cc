#include "../include/EventLoop.h"
#include "../include/Channel.h"
#include "../include/Poller.h"

#include <sys/eventfd.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <memory>


// 防止一个线程创建多个EventLoop
/*
    全局的指针，当第一个EventLoop对象创建起来后，他就指向这个对象
    再创建的时候发现不是nullptr，就不会在创建了
*/
__thread EventLoop* t_loopInThisThread = nullptr;

// 定义默认的Poller IO复用接口的超时时间
const int kPollTimeMs = 10000;

// 创建warnupfd，用来notify唤醒subReactor处理新来的channel
int createEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(evtfd < 0)
    {
        /* 一条日志：FATAL eventfd error:%d \n, errno */
    }
    return evtfd;
}

bool EventLoop::isInLoopThread() const
{
    return threadId_ == tid();
}


EventLoop::EventLoop():
    looping_(false),
    quit_(false),
    threadId_(tid()),
    poller_(Poller::newDefaultPoller(this)),
    // poller_(std::make_unique<Poller>()),
    wakeupFd_(createEventfd()),
    wakeupChannel_(std::make_unique<Channel>(this, wakeupFd_)),
    callingPendingFunctors_(false)
{
    /* 一条日志：DEBUG  EventLoop created %p in thread %d \n, this, threadId_ */
    if(t_loopInThisThread)
    {   // 如果该线程已经有一个Loop就通过FATAL exit()
        /* 一条日志，FATAL */
    }
    else
    {
        t_loopInThisThread = this;
    }

    /*
        每个subReactor都有一个wakeupChannel_，它绑定了一个简单的读回调函数，当
        mainReactor通过向对应的subReactor的wakeupChannel_发送消息，读回调函数
        接收到消息就会醒来，去处理mainReactor分发的任务。
    */

    // 设置wakeupfd的事件类型以及发生事件后的回调操作
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
    // 每一个eventloop都将监听wakeupchannel的EPOLLIN读事件了
    wakeupChannel_->enableReading();   // 只对读事件感兴趣
}

EventLoop::~EventLoop()
{
    wakeupChannel_->disableAll();  // 对所有的事件都不感兴趣
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

void EventLoop::handleRead()
{
    uint64_t one = 1;  // 发一个8字节的整数
    ssize_t n = read(wakeupFd_, &one, sizeof one);  // ?
    if(n != sizeof one)
    {
        /* 一条日志 ERROR Eventloop::handleRead() reads << n << bytes instead of 8*/
    }
}

void EventLoop::loop()
{
    looping_ = true;
    quit_ = false;

    /* 一条日志 INFO EventLoop %p start looping \n, this*/
    while(!quit_)
    {
        activateChannels_.clear();
        // 监听两类fd，  一种是client的fd，一种wakeupfd
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activateChannels_);
        for(Channel* channel: activateChannels_)
        {   // Poller能够监听那些channel发生事件了，然后上报该EventLoop，通知Channel处理相应的事件
            channel->handleEvent(pollReturnTime_);  
        }
        // 执行当前EventLoop事件循环需要处理的回调函数
        /*
            IO线程 mainLoop accept fd <= channel
            mainLoop只处理新用户的连接，已连接用户的Channel要发给subLoop

            mainLoop 事先注册一个回调函数cb(需要subloop来执行)
            wakeup subloop后，执行下面的方法，执行之前mainloop注册的cb
        */
        doPendingFunctors();  // 执行当前EventLoop事件循环需要处理的回调函数

        /*
            channel->handleEvent(pollReturnTime_);  处理的是每个发生了事件的Channel需要相应去处理的回调函数
            doPendingFunctors();  处理mainloop派发的回调操作，比如分配了新的Channel,就需要添加到列表，并注册到Poller中 
        */
    }

    /* 一条日志 INFO  EventLoop %p stop looping. \n, this*/
}


void EventLoop::quit()
{
    quit_ = true;
    if(!isInLoopThread())  // 如果是在其它线程中，调用的quit  在一个subloop中，调用了mainLoop的quit?
    {
        wakeup();
    }
}


// 在当前loop中执行cb
void EventLoop::runInLoop(Functor cb)
{   
    if(isInLoopThread())
    {
        cb();
    }
    else  // 在非当前loop线程中执行cb
    {   // 唤醒loop所在线程，执行cb
        queueInLoop(cb);
    }
}

// 把cb放入队列中，唤醒loop所在的线程，执行cb
void EventLoop::queueInLoop(Functor cb)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(cb);
    }

    /*  callingPendingFunctors_的含义
        当前loop正在执行回调，但刚给他添加了cb，需要他执行
        如果不wakeup，他执行完后机会在poll中阻塞住，刚添加的cb就无法立即执行
    */

    // 唤醒相应的，需要执行上面回调操作的loop的线程了
    if(!isInLoopThread() || callingPendingFunctors_)
    {
        wakeup();  // 唤醒loop所在线程
    }
}


// 用来唤醒loop所在的线程的 
/*
    通过wakeupFd_来唤醒对应线程
    即使对应线程不是处在epoll_wait中而是在执行下面的语句，也会在下一次循环执行到epoll_wait后立即返回
*/
void EventLoop::wakeup()
{   // 向wakeupfd_写一个数据，把它唤醒
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_, &one, sizeof one);
    if(n != sizeof one)
    {
        /* 一条日志 ERROR EventLoop::wakeup() writes %lu bytes instead of 8 \n, n*/
    }

}

// 用来唤醒loop的方法 => Poller的方法
void EventLoop::updateChannel(Channel* channel)
{
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
{
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel)
{
    return poller_->hasChannel(channel);
}

void EventLoop::doPendingFunctors()  // 执行回调
{   
    /*
        把pendingFunctors中的回调放到一个局部变量中
        交换完后，即便该线程还在执行回调，其他线程
        也能够向其pendingFunctors中写入
    */
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }
    // 执行回调
    for(const Functor& functor: functors)
    {
        functor();
    }
    callingPendingFunctors_ = false;
}

