//
// Created by py_01 on 26-7-9.
//

#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include"net/InetAddress.h"
#include"net/Buffer.h"
#include"protocol/MessageCodec.h"
#include<functional>
#include<memory>
#include<string>

class EventLoop;
class Channel;

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
public:
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
    using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
    using MessageCallback = std::function<void(const TcpConnectionPtr&, const std::string&)>;
    using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
    using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;

    TcpConnection(EventLoop* loop,
                    std::string  name,
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
    void connectDestroyed();

    void send(const std::string& message);
    void shutdown();

    void setConnectionCallback(ConnectionCallback cb) { connectionCallback_ = std::move(cb); }
    void setMessageCallback(MessageCallback cb) { messageCallback_ = std::move(cb); }
    void setCloseCallback(CloseCallback cb) { closeCallback_ = std::move(cb); }
    void setWriteCompleteCallback(WriteCompleteCallback cb) { writeCompleteCallback_ = std::move(cb); }

private:
    enum StateE{kConnecting, kConnected, kDisconnecting, kDisconnected};

    void setState(const StateE s) { state_ = s; }
    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError() const;
    void processInputBuffer();
    void sendInLoop(const std::string& message);
    void shutdownInLoop() const;

    EventLoop* loop_{nullptr};
    std::string name_;
    StateE state_;

    int sockfd_{-1};
    std::unique_ptr<Channel> channel_;

    const InetAddress localAddr_;
    const InetAddress peerAddr_;

    Buffer inputBuffer_;
    Buffer outputBuffer_;
    MessageCodec codec_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    CloseCallback closeCallback_;
    WriteCompleteCallback writeCompleteCallback_;
};

#endif //TCPCONNECTION_H
