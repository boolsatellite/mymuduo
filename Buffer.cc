#include "Buffer.h"
#include "errno.h"
#include <sys/uio.h>
/*
从fd上读取数据，Poller工作在LT模式下
Buffer缓冲区是有大小的，但是从fd上读取数据并不知道ecp数据的最终大小
*/
ssize_t Buffer::readFd(int fd , int* saveErrno)
{
    char extrabuf[65536];
    struct iovec vec[2];

    const size_t writable = writeableBytes();
    vec[0].iov_base = begin() + writable;
    vec[0].iov_len = writable;
    vec[1].iov_base = &extrabuf;
    vec[1].iov_len = sizeof(extrabuf);

    const int iovecnt = (writable < sizeof extrabuf) ? 2 : 1;

    const size_t n = readv(fd,vec,iovecnt);
    if(n < 0)
    {
        *saveErrno = errno;
    }
    else if(n < writable)
    {
        writerIndex_ += n;
    }
    else  //extrabuf中也写入了数据
    {
        writerIndex_ = buffer_.size();
        append(extrabuf,n - writable);
    }
    return n;
}