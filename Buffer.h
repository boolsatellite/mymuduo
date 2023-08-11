#pragma once

#include <vector>
#include <string>
#include <unistd.h>
// 网络库底层的缓冲区定义

/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size

class Buffer
{
public:
    static const size_t kCheapPrepend = 8;   // 记录数据包的长度
    static const size_t kInitialSize = 1024; // 数据包的大小

    Buffer(size_t initialSize = kInitialSize)
        : buffer_(kCheapPrepend + initialSize), readerIndex_(kCheapPrepend), writerIndex_(kCheapPrepend)
    {
    }

    ~Buffer() = default;

    size_t readableBytes() const
    {
        return writerIndex_ - readerIndex_;
    }

    size_t writeableBytes() const
    {
        return buffer_.size() - writerIndex_;
    }

    size_t prependableBytes() const
    {
        return readerIndex_;
    }

    const char* peek()    //返回可读数据的起始地址
    {
        return &(*(buffer_.begin() + readerIndex_));
    }

    void retrieve(size_t len)
    {
        if(len < readableBytes())  //数据没有读完，将readIndex指向未读数据
        {
            readerIndex_ += len;
        }
        else   //len == readableBytes()
        {
            retrieveAll();
        }
    }

    void retrieveAll()
    {
        readerIndex_ = writerIndex_ = kCheapPrepend;
    }

    //将onMessage上报的Buffer数据，转成string数据返回
    std::string retrieveAllAsAString()
    {
        return retriveAsString(readableBytes());
    }

    std::string retriveAsString(size_t len)
    {
        std::string result(peek() , len);  //将缓冲区可读的读取出来
        retrieve(len);                     //对缓冲区进行复位操作
        return result;
    }

    void ensureWriteableBytes(size_t len)
    {
        if(writeableBytes() < len)   //如果缓冲区中空间不足
        {
            makeSpace(len);            //扩容
        }
    }

    char* beginWrite()
    {
        return begin() + writerIndex_;
    }

    const char* beginWrite() const 
    {
        return begin() + writerIndex_;
    }

    //将[data,data+len]内存上的数据添加到buffer_缓冲区中
    void append(const char* data , size_t len)
    {
        ensureWriteableBytes(len);
        std::copy(data , data+len , beginWrite());
        writerIndex_ += len;
    }

    //从fd上读取数据
    ssize_t readFd(int fd , int* savedErrno);
    ssize_t writeFd(int fd , int* saveErrno);

private:
    char *begin()
    {
        return &*buffer_.begin();
    }

    const char* begin() const   //const 成员函数只能返回指向成员的const指针或引用。
    {
        return &(*buffer_.begin());
    }

    void makeSpace(size_t len)
    {
        if(writeableBytes() + prependableBytes() < len + kCheapPrepend)
        {
            buffer_.resize(writerIndex_ + len);
        }
        else
        {
            size_t readable = readableBytes();
            std::copy(begin()+readerIndex_ , begin() + writerIndex_ , begin() + kCheapPrepend);

        } 
    }

    std::vector<char> buffer_;
    size_t readerIndex_;   //可读数据的下标
    size_t writerIndex_;   //可写数据的下标
};