#ifndef THREAD_H
#define THREAD_H

#include "noncopyable.h"

#include <functional>
#include <thread>
#include <memory>
#include <unistd.h>
#include <string>
#include <atomic>


class Thread: noncopyable
{
public:
    using Threadfunc = std::function<void()>;

    explicit Thread(Threadfunc, const std::string& name = std::string());
    ~Thread();

    void start();
    void join();

    bool started() const;
    pid_t tid() const;
    const std::string& name() const;

    static int numCreated();

private:
    void setDefaultName();

    bool started_;
    bool joined_;

    std::shared_ptr<std::thread> thread_;
    pid_t tid_;
    Threadfunc func_;
    std::string name_;

    static std::atomic_int numCreated_;
};



#endif