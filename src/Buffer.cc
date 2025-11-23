#include "../include/Buffer.h"

#include <algorithm>
#include <sys/uio.h>
#include <unistd.h>

Buffer::Buffer(std::size_t initialSize):
    buffer_(kCheapPrepend + initialSize),
    readerIndex_(kCheapPrepend),
    writeIndex_(kCheapPrepend)
{

}

Buffer::~Buffer() = default;

std::size_t Buffer::readableBytes() const
{
    return writeIndex_ - readerIndex_;
}

std::size_t Buffer::writableBytes() const
{
    return buffer_.size() - writeIndex_;
}

std::size_t Buffer::prependableBytes() const
{
    return readerIndex_;
}

const char* Buffer::peek() const
{
    return begin() + readerIndex_;
}

void Buffer::retrieve(std::size_t len)
{   // onMessage string <- Buffer
    if(len < readableBytes())
    {
        readerIndex_ += len;  // 应用只读取了可读缓冲区数据的一部分
    }
    else
    {
        retrieveAll();
    }
}

void Buffer::retrieveAll()
{
    readerIndex_ = writeIndex_ = kCheapPrepend;
}

std::string Buffer::retrieveAsString(std::size_t len)
{
    std::string result(peek(), len);
    retrieve(len);
    return result;
}

std::string Buffer::retrieveAllAsString()
{
    return retrieveAsString(readableBytes());  // 应用可读取数据的长度
}   

void Buffer::ensureWritableBytes(std::size_t len)
{
    if(writableBytes() < len)
    {
        makeSpace(len);
    }
}

// 把[data, data+len]内存上的数据，添加到writable缓冲区中
void Buffer::append(const char* data, std::size_t len)
{
    ensureWritableBytes(len);
    std::copy(data, data+len, beginWrite());
    writeIndex_ += len;
}

char* Buffer::beginWrite()
{
    return begin() + writeIndex_;
}

const char* Buffer::beginWrite() const
{
    return begin() + writeIndex_;
}

char* Buffer::begin()
{
    return &*buffer_.begin();
}

const char* Buffer::begin() const
{
    return &*buffer_.begin();  // 数组的起始地址
}

void Buffer::makeSpace(std::size_t len)
{
    if(writableBytes() + prependableBytes() < len + kCheapPrepend)
    {
        buffer_.resize(writeIndex_ + len);
    }
    else
    {
        std::size_t readable = readableBytes();
        std::copy(begin() + readerIndex_,
                begin() + writeIndex_,
                begin() + kCheapPrepend
        );
        readerIndex_ = kCheapPrepend;
        writeIndex_ = readerIndex_ + readable;
    }
}

/*
    从fd上读取数据，Poller工作在LT模式
    Buffer缓冲区是有大小的，但是从fd上读数据的时候，却不知道tcp数据最终的大小
*/
ssize_t Buffer::readFd(int fd, int* saveErrno)
{
    char extrabuf[65536] = {0};  // 栈上的内存空间
    struct iovec vec[2];
    const ssize_t writable = writableBytes();  // 这是Buffer底层缓冲区剩余的可写空间大小
    vec[0].iov_base = begin() + writeIndex_;
    vec[0].iov_len = writable;

    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);

    const int iovcnt = (writable < sizeof(extrabuf) ? 2 : 1);
    const ssize_t n = ::readv(fd, vec, iovcnt);
    if(n < 0)
    {
        *saveErrno = errno; 
    }
    else if(n <= writable)  // buffer的可写缓冲区已经够存储读出来的数据
    {
        writeIndex_ += n;
        
    }
    else
    {
        writeIndex_ = buffer_.size();
        append(extrabuf, n - writable);  // writerIndex_开始写 
    }
    return n;
}

ssize_t Buffer::writeFd(int fd, int* saveErrno)
{
    ssize_t n = ::write(fd, peek(), readableBytes());
    if(n < 0)
    {
        *saveErrno = errno;
    }
    return n;
}