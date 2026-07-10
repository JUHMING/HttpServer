//
// Created by py_01 on 26-7-9.
//

#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include"net/InetAddress.h"
#include<memory>
#include<string>

class EventLoop;
class Socket;
class Channel;


class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
public:
    TcpConnection(EventLoop* loop,
                    const std::string& name,
                    int sockfd,
                    const InetAddress& localAddr,
                    const InetAddress& peerAddr);
    ~TcpConnection();

    EventLoop* getLoop() const {return loop_;}
    const std::string& getName() const {return name_;}
    const InetAddress& getLocalAddr() const {return localAddr_;}
    const InetAddress& getPeerAddr() const {return peerAddr_;}
    bool isConnected() const {return state_ == kConnected;}
    void connectEstablished();

private:
    enum StateE{kConnecting, kConnected, kDisconnecting, kDisconnected};

    void setState(StateE s) { state_ = s; }
    void handleRead();

    EventLoop* loop_;
    std::string name_;
    StateE state_;

    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    const InetAddress localAddr_;
    const InetAddress peerAddr_;
};

#endif //TCPCONNECTION_H
