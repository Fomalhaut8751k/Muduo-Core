#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include <string>

// 网络库底层的缓冲区的定义

/*
@code
+-------------------+-----------------+-----------------+
| prependable bytes |  readable bytes |  writable bytes |
|                   |    (CONTENT)    |                 |
+-------------------+-----------------+-----------------+
|                   |                 |                 |
0       <=     readerIndex  <=   writerIndex   <=      size
@endcode
*/

class Buffer
{
public:
    static const std::size_t kCheapPrepend = 8;
    static const std::size_t kInitialSize = 1024;

    explicit Buffer(std::size_t initialSize = kInitialSize);
    ~Buffer();

    std::size_t readableBytes() const;
    std::size_t writableBytes() const;
    std::size_t prependableBytes() const;

    const char* peek() const;

    void retrieve(std::size_t len);
    void retrieveAll();

    std::string retrieveAsString(std::size_t len);
    std::string retrieveAllAsString();

    void ensureWritableBytes(std::size_t len);

    void append(const char* data, std::size_t len);

    char* beginWrite();
    const char* beginWrite() const;

    // 从fd上读取数据
    ssize_t readFd(int fd, int* saveErrno);

private:
    char* begin();
    const char* begin() const;

    std::vector<char> buffer_;
    std::size_t readerIndex_;
    std::size_t writeIndex_;

    void makeSpace(std::size_t len);
};


#endif