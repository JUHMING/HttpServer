//
// Created by py_01 on 26-7-9.
//

#include"TcpConnection.h"
#include"event/EventLoop.h"
#include"event/Channel.h"
#include"log/Logger.h"
#include"unistd.h"
#include<cstring>
#include<fcntl.h>
#include<iostream>
#include <utility>

namespace {
    void setNonBlockAndCloseOnExec(int fd) {
        int flags = ::fcntl(fd, F_GETFL, 0);
        if (flags >= 0) {
            ::fcntl(fd, F_SETFL, flags | O_NONBLOCK);
        }

        flags = ::fcntl(fd, F_GETFD, 0);
        if (flags >= 0) {
            ::fcntl(fd, F_SETFD, flags | FD_CLOEXEC);
        }
    }
}

TcpConnection::TcpConnection(EventLoop *loop,
                            std::string name,
                            int sockfd,
                            const InetAddress &localAddr,
                            const InetAddress &peerAddr)
    : loop_(loop),
    name_(std::move(name)),
    state_(kConnecting),
    sockfd_(sockfd),
    channel_(std::make_unique<Channel>(sockfd, loop)),
    localAddr_(localAddr),
    peerAddr_(peerAddr)
{
    setNonBlockAndCloseOnExec(sockfd_);
    channel_->setReadCallback([this] { handleRead(); });
    channel_->setWriteCallback([this] { handleWrite(); });
    channel_->setCloseCallback([this] { handleClose(); });
    channel_->setErrorCallback([this] { handleError(); });
}

TcpConnection::~TcpConnection() {
    std::string msg = "TcpConnection::dtor [" + name_ + "] at fd " + std::to_string(sockfd_) + "\n";
    Logger::debug(msg);
    if (sockfd_ != -1) {
        ::close(sockfd_);
        sockfd_ = -1;
    }
}

void TcpConnection::connectEstablished() {
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading();
    if (connectionCallback_) {
        connectionCallback_(shared_from_this());
    }
}

void TcpConnection::connectDestroyed() {
    if (state_ == kConnected) {
        setState(kDisconnected);
        channel_->disableAll();
        if (connectionCallback_) {
            connectionCallback_(shared_from_this());
        }
    }
    channel_->remove();
}

void TcpConnection::send(const std::string& message) {
    if (state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(message);
        } else {
            TcpConnectionPtr self = shared_from_this();
            loop_->queueInLoop([self, message] {
                self->sendInLoop(message);
            });
        }
    }
}

void TcpConnection::shutdown() {
    if (state_ == kConnected) {
        setState(kDisconnecting);
        TcpConnectionPtr self = shared_from_this();
        loop_->runInLoop([self]{
            self->shutdownInLoop();
        });
    }
}

void TcpConnection::shutdownInLoop() const {
    if (channel_->isWriteEvent()) {
        ::shutdown(sockfd_, SHUT_WR);
    }
}

void TcpConnection::handleRead() {
    int savedError = 0;
    ssize_t n = inputBuffer_.readFd(sockfd_, &savedError);
    if (n > 0) {
        processInputBuffer();
    } else if (n == 0) {
        handleClose();
    } else {
        if ((savedError == EAGAIN) || savedError == EWOULDBLOCK) {
            return;
        }
        if (savedError == ECONNRESET) {
            handleClose();
            return;
        }
        errno = savedError;
        handleError();
    }
}

void TcpConnection::processInputBuffer() {
    while (true) {
        std::string message;
        const auto status = codec_.tryDecode(&inputBuffer_, message);

        if (status == MessageCodec::DecodeStatus::kSuccess) {
            if (messageCallback_) {
                messageCallback_(shared_from_this(), message);
            }
            continue;
        }

        if (status == MessageCodec::DecodeStatus::kNeedMoreData) {
            break;
        }

        std::string msg = "TcpConnection::processInputBuffer invalid message length in [" + name_ + "]";
        Logger::error(msg);
        handleClose();
        break;
    }
}

void TcpConnection::handleWrite() {
    if (!channel_->isWriteEvent()) {
        return;
    }

    ssize_t n = ::write(sockfd_, outputBuffer_.peek(), outputBuffer_.readableBytes());
    int savedErrno = (n < 0) ? errno : 0;

    if (n > 0) {
        outputBuffer_.retrieve(static_cast<size_t>(n));
        if (outputBuffer_.readableBytes() == 0) {
            channel_->disableWriting();
            if (writeCompleteCallback_) {
                TcpConnectionPtr self = shared_from_this();
                loop_->queueInLoop([self] {
                    if (self->writeCompleteCallback_) {
                        self->writeCompleteCallback_(self);
                    }
                });
            }
            if (state_ == kDisconnecting) {
                shutdownInLoop();
            }
        }
    } else if (n < 0) {
        if (savedErrno != EAGAIN && savedErrno != EWOULDBLOCK) {
            handleError();
        }
    }
}

void TcpConnection::handleClose() {
    setState(kDisconnected);
    channel_->disableAll();

    TcpConnectionPtr self = shared_from_this();
    if (connectionCallback_) {
        connectionCallback_(self);
    }

    if (closeCallback_ ) {
        closeCallback_(self);
    }
}

void TcpConnection::handleError() const {
    int error = 0;
    socklen_t len = sizeof(error);
    if (::getsockopt(sockfd_, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
        error = errno;
    }
    std::string msg = " TcpConnection::handleError [" + name_
          + "] errno=" + std::to_string(error) + " " + std::strerror(error);
    Logger::error(msg);
}

void TcpConnection::sendInLoop(const std::string& message) {
    if (state_ == kDisconnected) {
        return;
    }

    const std::string packet = codec_.encode(message);
    ssize_t nwrote = 0;
    size_t remaining = packet.size();
    bool faultError = false;

    if (!channel_->isWriteEvent() && outputBuffer_.readableBytes() == 0) {
        nwrote = ::write(sockfd_, packet.data(), packet.size());
        if (nwrote >= 0) {
            remaining = packet.size() - static_cast<size_t>(nwrote);
            if (remaining == 0 && writeCompleteCallback_) {
                TcpConnectionPtr self = shared_from_this();
                loop_->queueInLoop([self] {
                    if (self->writeCompleteCallback_) {
                        self->writeCompleteCallback_(self);
                    }
                });
            }
        } else {
            nwrote = 0;
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                if (errno == EPIPE || errno == ECONNRESET) {
                    faultError = true;
                }
                handleError();
            }
        }
    }

    if (!faultError && remaining > 0) {
        outputBuffer_.append(packet.data() + nwrote, remaining);
        if (!channel_->isWriteEvent()) {
            channel_->enableWriting();
        }
    }

}
