# Muduo-Core
muduo网络库的核心功能

1. 2025.10.29

    熟悉了TCP网络编程通信的一些重要方法: `socket()`, `bind()`, `listen()`, `accept()`, `send()`, `recv()`, `close()`.

2. 2025.10.31

    熟悉了epoll重要函数如`epoll_create()`, `epoll_wait()`, `epoll_ctl()`的使用，以及epoll和socket结合实现简单的服务端与客户端的通信功能。

3. 2025.11.2

    熟悉了UTP网络编程，TCP协议和UDP协议，TCP协议的三次挥手和四次挥手等

4. 2025.11.3

    熟悉了IO的同步和异步，Unix/Linux的五种IO模型，Reactor模型

5. 2025.11.4 ~ 2025.11.7

    完成了`Channel`, `Poller`, `EpollPoller`, `InetAddress`, `TimeStamp`代码的书写
    
    将异步日志系统集成到该项目中

6. 2025.11.8 ~ 2025.11.21

    完成了`EventLoop`, `Thread`, `EventLoopThread`, `EventLoopThreadPool`, `Acceptor`等代码的编写

7. 2025.11.22 ~ 2025.11.23

    完成了剩余代码的编写，实现了测试用例


<br>

## Channel, Poller和EventLoop

简单的基于`epoll`的代码如下，涉及到`epoll`的三个主要函数`epoll_create()`, `epoll_ctl()`以及`epoll_wait()`：
![](img/epoll.png)

### (1) epoll_create()
这个直接就在`Poller`的构造函数中调用：
```cpp
EpollPoller::EpollPoller(EventLoop* loop):
    Poller(loop),
    epollfd_(epoll_create1(EPOLL_CLOEXEC)), 
    events_(KInitEventListSize)
{
    ...
```
### (2) epoll_ctl()
`epoll_ctl()`在`Muduo`网络库的调用方式如下，对于`Channel`可以把它类比成`struct epoll_event`，它们都封装了`fd`和`event`:
```cpp
struct epoll_event
{
  uint32_t events;  /* Epoll events */
  epoll_data_t data;    /* User data variable */
} __EPOLL_PACKED;

typedef union epoll_data
{
  void *ptr;
  int fd;
  uint32_t u32;
  uint64_t u64;
} epoll_data_t;
```
![](img/epoll_ctl.png)
`Channel`想把自己添加到`epoll`实例，或者从实例中删除，他自己是无法做到，但是它的成员变量中记录了它所在的`EventLoop`，`EventLoop`中又记录了`Poller`指针，通过`EventLoop`再到`Poller`就可以执行到`epoll_ctl()`

### (3) epoll_wait()

在`EventLoop`和`Poller`中，都有记录`Channel`的容器，但功能上不一样，`Poller`中记录的注册即`epoll_ctl()`了的`Channel`，如果`__op`是添加，就加入到其中；而`EventLoop`的记录的是处于活跃状态的`Channel`，哪些`Channel`是活跃的`EventLoop`是事先不知道的，只有`epoll_wait()`，也就是`Poller`调用`poll()`后才知道，`EventLoop`通过`poller_`调用`poll()`时会把自己的这个`Channel`容器作为参数传入，`Poller`会把处于活跃状态的`Channel`写入到这个容器中，这样`EventLoop`就知道了。

![](img/epoll_wait.png)

<br>

## EventLoop, Thread, EventLoopThread和EventLoopThreadPool
### (1) Thread
对`std::thread`的简单封装，记录了所在的线程号，分离了线程的创建和启动，避免了`std::thread t(threadfunc)`直接启动线程的问题。

### (2) EventLoopThread
对`Thread`的封装，除此之外的成员变量还有`EventLoop`，一个`EventLoop`对应一个`Thread`，实现了`one Loop per thread`的设计理念。`EventLoop`创建在`EventLoopThread`提供给`Thread`的线程函数中，随着线程结束被释放掉。这个线程函数本质上就是启动了一个事件循环。
```cpp
void EventLoopThread::threadFunc()
{
    EventLoop loop; 
    ...
    loop.loop();
    ...
} 
```
![](img/EventLoop_and_Thread.png)

### (3) EventLoopThreadPool
`EventLoopThreadPool`中包含了一个`mainloop`和若干个`subloop`，在`Muduo`网络库中，`mainloop`用于处理连接事件，而其他的读写事件都交给`subloop`处理。`baseLoop_`通过构造函数的参数初始化，`EventLoopThread`的事件循环会创建一个`EventLoop`并返回，然后记录在`loops_`中。
```cpp
EventLoop* baseLoop_;  
std::vector<EventLoop*> loops_;
```

<br>

## 从Acceptor()看事件循环的触发调用逻辑
为`Acceptor`的成员变量`acceptChannel_`设置回调函数。
![](img/set_callback.png)
之前写的`epoll`代码，`epoll`实例都只对用户写入事件感兴趣。使`epoll`实例对用户连接事件感兴趣，只要把处理连接的套接字`server_fd_`对应的事件通过`epoll_ctl()`进行添加，这样`epoll`实例`epfd_`就会监听用户的连接事件。
```cpp
events[0].data.fd = server_fd;
events[0].events = EPOLLIN;
epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &events[0]);
...
if(events[i].data.fd == server_fd)
{
    std::cout << "有新链接" << std::endl;
...
```
![](img/run_callback.png)

在`Acceptor`的`acceptChannel_`所属的事件循环就是`mainloop`，负责监听和处理用户的连接活动。`acceptChannel_`会被注册到`Acceptor::poller_`中，并只对用户连接事件感兴趣。`EventLoop::loop()`调用，执行`poller_->poll()`，`poll()`中执行`epoll_wait()`等待事件发生。当有新用户连接时，`epoll_wait()`就会返回，并把处于活跃状态的`Channel`写到`Eventloop`的`activeChannels_`中，`EventLoop`遍历并对调用`Channel`的回调函数。`Acceptor`的回调函数中包含了`accept()`操作，由于有新用户发起连接，因此`accept()`不会阻塞会立即回用户分配一个`fd`。


## Buffer

初始状态的下的缓冲区如图所示：
![](img/init_buffer.png)

读取缓冲区可读区域的数据有两个函数：
```cpp
std::string retrieveAsString(std::size_t len);
std::string retrieveAllAsString();
```
![](img/get_from_buffer.png)

写入缓冲区可能需要扩容

缓冲区扩容，也有两种情况：如果调整缓冲区的结构可以空出一片足够长度的连续空间，就不需要扩容，如果不够再扩容
![](img/makespace.png)
