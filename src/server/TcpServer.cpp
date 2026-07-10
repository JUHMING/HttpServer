//
// Created by py_01 on 26-7-9.
//

#include"TcpServer.h"

#include <iostream>

#include"Acceptor.h"
#include"connection/TcpConnection.h"
#include"event/EventLoop.h"

TcpServer::TcpServer(EventLoop *loop, const InetAddress &addr, const std::string& name)
    : loop_(loop),
        name_(name),
        ipPort_(addr.toString()),
        acceptor_(std::make_unique<Acceptor>(addr.getIp(), addr.getPort())),
        nextConnId_(1)
{
    acceptor_->setNewConnectionCallback(
        [this](const int sockfd, const InetAddress& peerAddr) {
            this->newConnection(sockfd, peerAddr);
        }
    );
}

TcpServer::~TcpServer() {
    //[ALTER]
    std::cout << "[INFO]: TcpServer::~TcpServer [" << name_ << "] destorying..." << std::endl;
}

void TcpServer::start() const {
    acceptor_->listen();
    std::cout << "[INFO]: TcpServer::start -> [" << name_ << "] is now listening on " << ipPort_ << std::endl;
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr) {
    std::string connName = name_ + "-" + peerAddr.toString() + "#" + std::to_string(nextConnId_++);
    //[ALTER]
    std::cout << "[INFO]: TcpServer::newConnection -> New connection [" << connName
              << "] from " << peerAddr.toString() << " assigned to fd " << sockfd << std::endl;

    //[WARNING]
    struct sockaddr_in local_addr{};
    socklen_t addr_len = sizeof(local_addr);
    if (::getsockname(sockfd, reinterpret_cast<struct sockaddr*>(&local_addr), &addr_len) < 0) {
        std::cerr << "[ERROR]: getsockname failed in TcpServer::newConnection" << std::endl;
    }
    InetAddress localAddr(local_addr);
    //[WARNING]

    auto conn = std::make_shared<TcpConnection>(loop_, connName, sockfd, localAddr, peerAddr);
    connectionMap_[connName] = conn;
    conn->connectEstablished();
}




























