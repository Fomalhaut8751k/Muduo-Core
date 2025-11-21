#include "../include/Thread.h"
#include "../include/CurrentThread.h"

#include <semaphore.h>

std::atomic_int Thread::numCreated_(0);

Thread::Thread(Threadfunc func, const std::string& name):
    func_(std::move(func)),
    name_(name),
    thread_(nullptr),
    tid_(0),
    started_(false),
    joined_(false)
{
    setDefaultName();
}

Thread::~Thread()
{
    if(started_ && !joined_)  // join() 和 detach() 不能同时调用
    { 
        thread_->detach();
    }
}

void Thread::start()  
{
    if(!thread_ && !started_)
    {
        started_ = true;

        sem_t sem;
        sem_init(&sem, false, 0);

        // thread_ = std::make_shared<std::thread>(func_);
        thread_ = std::shared_ptr<std::thread>(new std::thread(
            [&]()->void{
                tid_ = tid();  // 获取线程的tid值
                sem_post(&sem);
                func_();
            }
        ));

        // 这里必须等待获取上面新创建的线程的tid值
        sem_wait(&sem);  // 如果sem_post()还没被执行，就会被阻塞在这里
    }
    
}

void Thread::join()
{
    if(started_ && !joined_)
    {
        joined_ = true;
        thread_->join();
    }
}

void Thread::setDefaultName()
{
    int num = ++numCreated_;
    if(name_.empty())
    {   
        char buf[32] = {0};
        snprintf(buf, sizeof buf, "Thread%d", num);
        name_ = buf;
    }
}

bool Thread::started() const { return started_; }

pid_t Thread::tid() const { return tid_; }

const std::string& Thread::name() const { return name_; }

int Thread::numCreated() { return numCreated_; }