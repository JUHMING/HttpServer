//
// Created by py_01 on 26-7-9.
//

#ifndef TCPSERVER_H
#define TCPSERVER_H

#include"net/InetAddress.h"
#include<memory>
#include<string>
#include<unordered_map>

class EventLoop;
class Acceptor;
class TcpConnection;

class TcpServer {
public:
    TcpServer(EventLoop* loop, const InetAddress& peerAddr, const std::string& name);
    ~TcpServer();
    void start() const;

private:
    void newConnection(int sockfd, const InetAddress& peerAddr);

    EventLoop* loop_;
    const std::string name_;
    const std::string ipPort_;

    std::unique_ptr<Acceptor> acceptor_;

    using ConnectionMap = std::unordered_map<std::string, std::shared_ptr<TcpConnection>>;
    ConnectionMap connectionMap_;

    int nextConnId_; //used to generate auto-incrementing id
};

#endif //TCPSERVER_H
