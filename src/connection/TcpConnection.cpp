//
// Created by py_01 on 26-7-9.
//

#include"TcpConnection.h"
#include"event/EventLoop.h"
#include"event/Channel.h"
#include"net/Socket.h"
#include"unistd.h"
#include<cstring>
#include<iostream>

TcpConnection::TcpConnection(EventLoop *loop,
                            const std::string &name,
                            int sockfd,
                            const InetAddress &localAddr,
                            const InetAddress &peerAddr)
    : loop_(loop),
    name_(name),
    state_(kConnecting),
    socket_(std::make_unique<Socket>()),
    channel_(std::make_unique<Channel>(sockfd, loop)),
    localAddr_(localAddr),
    peerAddr_(peerAddr)
{
    channel_->setReadCallback([this] { handleRead(); });
}

TcpConnection::~TcpConnection() {
    //[ALTER]
    std::cout << "[DEBUG]: TcpConnection::dtor [" << name_ << "] at fd " << socket_->fd() << std::endl;
}

void TcpConnection::connectEstablished() {
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading();
}

//[WARNING]
void TcpConnection::handleRead() {
    char buf[1024] = {};

    int fd = channel_->fd();

    ssize_t n = ::read(fd, buf, sizeof(buf) - 1);

    if (n > 0) {
        std::cout << "[INFO]: TcpConnection::handleRead -> Received " << n
                  << " bytes from client [" << name_ << "]: " << buf << std::endl;
        ::write(fd, buf, n);
    }
    else if (n == 0) {
        std::cout << "[INFO]: TcpConnection::handleRead -> Client [" << name_
                  << "] closed connection (fd: " << fd << ")" << std::endl;
    }
    else {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            std::cerr << "[ERROR]: TcpConnection::handleRead -> read error on fd "
                      << fd << ", errno: " << errno << std::endl;
        }
    }
}
