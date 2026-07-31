//
// Created by py_01 on 26-7-10.
//

#include"Buffer.h"
#include <sys/uio.h>
#include <unistd.h>
#include <cerrno>

ssize_t Buffer::readFd(const int fd, int *savedErrno) {
    long size = 65536 ;
    char buffer[size];
    struct iovec vec[2];

    const size_t writable = writableBytes();

    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writable;

    vec[1].iov_base = buffer;
    vec[1].iov_len = sizeof(buffer);

    const int iovcnt = (writable < sizeof(buffer)) ? 2 : 1;
    const ssize_t n = ::readv(fd, vec, iovcnt);

    if (n < 0) {
        *savedErrno = errno;
    }else if (static_cast<size_t>(n) <= writable) {
        writerIndex_ += n;
    }else {
        writerIndex_ = buffer_.size();
        append(buffer, n - writable);
    }
    return n;
}

void Buffer::makeSpace(const size_t len)
{
    if (writableBytes() + prependableBytes() < len)
    {
        size_t newSize =
            std::max(
                buffer_.size()*2,
                writerIndex_ + len
            );

        buffer_.resize(newSize);
    }
    else
    {
        size_t readable = readableBytes();

        std::copy(
            begin()+readerIndex_,
            begin()+writerIndex_,
            begin()+kCheapPrepend
        );

        readerIndex_=kCheapPrepend;
        writerIndex_=readerIndex_+readable;
    }
}
