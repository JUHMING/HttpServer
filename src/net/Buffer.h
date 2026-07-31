//
// Created by py_01 on 26-7-10.
//

#ifndef BUFFER_H
#define BUFFER_H

#include<vector>
#include<string>
#include<algorithm>
#include<cstddef>

class Buffer {
public:
    ///reserved area for low-cost head-end equipment
    ///It enables the application layer to append data to the protocol header
    ///at extremely low cost when assembling or sending network packets.
    static constexpr size_t kCheapPrepend = 8;

    static constexpr size_t kInitialSize = 1024; // initial read/write capacity

    explicit Buffer(const size_t size = kInitialSize)
            :buffer_(kCheapPrepend + size),
                readerIndex_(kCheapPrepend),
                writerIndex_(kCheapPrepend) {}

    [[nodiscard]] size_t readableBytes() const {return writerIndex_ - readerIndex_;}
    [[nodiscard]] size_t writableBytes() const {return buffer_.size() - writerIndex_;}
    [[nodiscard]] size_t prependableBytes() const {return readerIndex_;}

    // return the starting memory address of the readable data
    [[nodiscard]] const char* peek() const {return begin() + readerIndex_; }

    //extract data of a specified length
    void retrieve(const size_t len) {
        if (len < readableBytes()) {
            readerIndex_ += len;
        } else {
            retrieveAll();
        }
    }

    void retrieveAll() {
        readerIndex_ = kCheapPrepend;
        writerIndex_ = kCheapPrepend;
    }

    [[nodiscard]] std::string retrieveAllString() {
        return retrieveString(readableBytes());
    }

    [[nodiscard]] std::string retrieveString(const size_t len) {
        if (len > readableBytes())
            throw std::out_of_range("retrieveString: len exceeds readable bytes");
        std::string result(peek(), len);
        retrieve(len);
        return result;
    }

    void append(const char* data,const size_t len) {
        ensureWritableBytes(len);
        std::copy_n(data, len, beginWrite());
        hasWritten(len);
    }

    void ensureWritableBytes(const size_t len) {
        if (writableBytes() < len) {
            makeSpace(len);
        }
    }

    char* beginWrite() { return begin() + writerIndex_; }
    [[nodiscard]] const char* beginWrite() const { return begin() + writerIndex_; }

    void hasWritten(const size_t len) { writerIndex_ += len; }

    ssize_t readFd(int fd, int* savedErrno);

private:
    char* begin() {
        return &*buffer_.begin();
    }

    [[nodiscard]] const char* begin() const {
        return &*buffer_.begin();
    }

    // void makeSpace(const size_t len) {
    //     if (writableBytes() + prependableBytes() < len) {
    //         buffer_.resize(writerIndex_ + len);
    //     }else {
    //         size_t readable = readableBytes();
    //         std::copy(begin() + readerIndex_,
    //                   begin() + writerIndex_,
    //                   begin() + kCheapPrepend);
    //         readerIndex_ = kCheapPrepend;
    //         writerIndex_ = readerIndex_ + readable;
    //     }
    // }

    void makeSpace(size_t len);

    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;
};

#endif //BUFFER_H
