//
// Created by py_01 on 26-6-28.
//

#include"Acceptor.h"
#include <unistd.h>

Acceptor::Acceptor(std::string_view ip, uint16_t port):
                listenSocket_(),
                acceptAddress_(std::string(ip), port)
{
    listenSocket_.bind(acceptAddress_);
}

Acceptor::Acceptor(const uint16_t port) :
                listenSocket_(),
                acceptAddress_(port)
{
    listenSocket_.bind(acceptAddress_);
}

void Acceptor::listen() const {
    listenSocket_.listen();
}

void Acceptor::handleRead() const {
    auto [client_fd, client_addr] = listenSocket_.accept();
    if (newConnectionCallback_) {
        newConnectionCallback_(client_fd, client_addr);
    }else {
        ::close(client_fd);
    }
}