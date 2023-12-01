//
// Created by satellite on 2023-11-24.
//

#include <cassert>
#include <sys/uio.h>
#include <errno.h>
#include <string>
#include <sys/socket.h>
#include "Buffer.h"

Buffer::Buffer(size_t initialSize)
    : buffer_(kcheapPrend + initialSize)
    , readerIndex_(kcheapPrend)
    , writerIndex_(kcheapPrend)
{
    assert(readableBytes() == 0);
    assert(writableBytes() == initialSize);
    assert(prependableBytes() == kcheapPrend);
}

/*
 * buffer缓冲区是有读取上限的，但是从fd上读取数据时，不知道数据的长度
 */
ssize_t Buffer::readFd(int fd, int *savedErrno) {
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writeable = writableBytes();
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writeable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;

    const int iovcnt = (writeable < sizeof extrabuf) ? 2 : 1;
    const ssize_t n = ::readv(fd,vec,iovcnt);
    if (n < 0) {
        *savedErrno = errno;
    } else if (n <= writeable) {
        writerIndex_ += n;
    } else {
        writerIndex_ = buffer_.size();
        append(extrabuf , n - writeable);
    }
    return n;
}



