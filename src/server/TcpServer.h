//
// Created by py_01 on 26-7-9.
//

#ifndef TCPSERVER_H
#define TCPSERVER_H

#include"net/InetAddress.h"
#include"connection/TcpConnection.h"
#include"thread/ThreadPool.h"
#include<cstddef>
#include<functional>
#include<memory>
#include<string>
#include<unordered_map>
#include<utility>

class EventLoop;
class Acceptor;
class TcpConnection;

class TcpServer {
public:
    using ThreadInitCallback = std::function<void()>;
    using ConnectionCallback = TcpConnection::ConnectionCallback;
    using MessageCallback = TcpConnection::MessageCallback;
    using WriteCompleteCallback = TcpConnection::WriteCompleteCallback;

    TcpServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& name);
    ~TcpServer();
    void start() ;

    void setConnectionCallback(ConnectionCallback cb){ connectionCallback_ = std::move(cb); }
    void setMessageCallback(MessageCallback cb){ messageCallback_ = std::move(cb); }
    void setWriteCompleteCallback(WriteCompleteCallback cb){ writeCompleteCallback_ = std::move(cb); }
    void setThreadNum(size_t numThread);
    void runTask(ThreadPool::Task task);
private:
    void newConnection(int sockfd, const InetAddress& peerAddr);
    void removeConnection(const std::shared_ptr<TcpConnection>& conn);
    void removeConnectionInLoop(const std::shared_ptr<TcpConnection>& conn);

    EventLoop* loop_;
    const std::string name_;
    const std::string ipPort_;

    std::unique_ptr<Acceptor> acceptor_;
    std::unique_ptr<Channel> acceptChannel_;
    ThreadPool threadPool_;

    using ConnectionMap = std::unordered_map<std::string, std::shared_ptr<TcpConnection>>;
    ConnectionMap connectionMap_;

    int nextConnId_; //used to generate auto-incrementing id
    bool started_{false};

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
};

#endif //TCPSERVER_H
