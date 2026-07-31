//
// Created by py_01 on 26-7-9.
//

#include"TcpServer.h"

#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <utility>

#include"Acceptor.h"
#include"event/Channel.h"
#include"event/EventLoop.h"

TcpServer::TcpServer(EventLoop *loop, const InetAddress &addr, const std::string& name)
    : loop_(loop),
        name_(name),
        ipPort_(addr.toString()),
        acceptor_(std::make_unique<Acceptor>(addr.getIp(), addr.getPort())),
        acceptChannel_(std::make_unique<Channel>(acceptor_->fd(), loop)),
        nextConnId_(1)
{
    acceptor_->setNewConnectionCallback(
        [this](const int sockfd, const InetAddress& peerAddr) {
            this->newConnection(sockfd, peerAddr);
        }
    );
    acceptChannel_->setReadCallback([this] {
        acceptor_->handleRead();
    });

    connectionCallback_ = [](const std::shared_ptr<TcpConnection>& conn) {
        std::cout << "[INFO]: connection " << conn->getName()
                  << (conn->isConnected() ? " UP" : " DOWN") << std::endl;
    };
    messageCallback_ = [](const std::shared_ptr<TcpConnection>& conn, const std::string& message) {
        conn->send(message);
    };
}

TcpServer::~TcpServer() {
    std::cout << "[INFO]: TcpServer::~TcpServer [" << name_ << "] destroying..." << std::endl;
    for (auto& item : connectionMap_) {
        item.second->connectDestroyed();
    }
    if (acceptChannel_) {
        acceptChannel_->disableAll();
        acceptChannel_->remove();
    }
    threadPool_.stop();
}

void TcpServer::start() {
    if (started_) {
        return;
    }

    started_ = true;
    threadPool_.start();
    acceptor_->listen();
    acceptChannel_->enableReading();
    std::cout << "[INFO]: TcpServer::start -> [" << name_ << "] is now listening on " << ipPort_ << std::endl;
}

void TcpServer::setThreadNum(const size_t numThread) {
    threadPool_.setThreadNum(numThread);
}

void TcpServer::runTask(ThreadPool::Task task) {
    threadPool_.run(std::move(task));
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr) {
    if (sockfd < 0) {
        return;
    }

    std::string connName = name_ + "-" + peerAddr.toString() + "#" + std::to_string(nextConnId_++);
    std::cout << "[INFO]: TcpServer::newConnection -> New connection [" << connName
              << "] from " << peerAddr.toString() << " assigned to fd " << sockfd << std::endl;

    struct sockaddr_in local_addr{};
    socklen_t addr_len = sizeof(local_addr);
    if (::getsockname(sockfd, reinterpret_cast<struct sockaddr*>(&local_addr), &addr_len) < 0) {
        std::cerr << "[ERROR]: getsockname failed in TcpServer::newConnection" << std::endl;
    }
    InetAddress localAddr(local_addr);

    auto conn = std::make_shared<TcpConnection>(loop_, connName, sockfd, localAddr, peerAddr);
    connectionMap_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback([this](const std::shared_ptr<TcpConnection>& connection) {
        removeConnection(connection);
    });
    conn->connectEstablished();
}

void TcpServer::removeConnection(const std::shared_ptr<TcpConnection>& conn) {
    loop_->queueInLoop([this, conn] {
        removeConnectionInLoop(conn);
    });
}

void TcpServer::removeConnectionInLoop(const std::shared_ptr<TcpConnection>& conn) {
    connectionMap_.erase(conn->getName());
    conn->connectDestroyed();
}



