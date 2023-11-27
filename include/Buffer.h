//
// Created by satellite on 2023-11-24.
//

#ifndef MYMUDUO_BUFFER_H
#define MYMUDUO_BUFFER_H

#include <vector>
#include <string>


/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode

class Buffer {
public:
    static const size_t kcheapPrend = 8;
    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t initialSize = kInitialSize);

    size_t readableBytes() const
    { return writerIndex_ - readerIndex_; }

    size_t writableBytes() const
    { return buffer_.size() - writerIndex_; }

    size_t prependableBytes() const
    { return readerIndex_; }

    char *peek() {                  //返回可写位置的指针
        return begin() + readerIndex_;
    }

    const char *peek() const {
        return begin() + readerIndex_;
    }

    char* beginWrite() {
        return begin() + writerIndex_;
    }

    const char* beginWrite() const {
        return begin() + writerIndex_;
    }

    void retrieveAll() {
        readerIndex_ = kcheapPrend;
        writerIndex_ = kcheapPrend;
    }

    std::string retrieveAllAsString() {
        retrieveAsString(readableBytes());
    }


    std::string retrieveAsString(size_t len) {
        assert(len < readableBytes());
        std::string result(peek() , len);
        retrieve(len);
        return result;
    }

    void retrieve(size_t len) {
        assert(len <= readerIndex_) ;
        if(len < readableBytes()) {
            readerIndex_ += len;
        } else {
            retrieveAll();
        }
    }


    void ensureWriteableBytes(size_t len) {
        if(writableBytes() < len) {
            makeSpace(len);
        }
        assert(writableBytes() >= len);
    }

    void makeSpace(size_t len) {
        if(prependableBytes() + writableBytes() < len + kcheapPrend) {
            buffer_.resize(writerIndex_ + len);
        } else {
            assert(kcheapPrend< readerIndex_);
            size_t readable = readableBytes();
            std::copy(begin() + readerIndex_ ,
                      begin() + writerIndex_ ,
                      begin() + kcheapPrend);
            readerIndex_ = kcheapPrend;
            writerIndex_ = readerIndex_ + readable;
            assert(readable == readableBytes());
        }
    }

    void append(const char* data , size_t len) {    //将data添加至writeable缓冲区中
        ensureWriteableBytes(len);
        std::copy(data , data + len , beginWrite());
        assert(len < writableBytes());
        writerIndex_ += len;
    }

    ssize_t readFd(int fd , int* savedErrno);       //从fd上读取数据

private:

    char* begin() {
        return &*buffer_.begin();
    }

    const char* begin() const {
        return &*buffer_.begin();
    }




    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;

    const static char kCRLF[];      //零长数组
};


#endif //MYMUDUO_BUFFER_H
