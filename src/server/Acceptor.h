//
// Created by py_01 on 26-6-27.
//

#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include"net/InetAddress.h"
#include"net/Socket.h"
#include<functional>
class Acceptor {
public:
    using NewConnectionCallback = std::function<void(int, const InetAddress&)>;

    Acceptor(std::string_view ip, uint16_t port);
    explicit Acceptor(uint16_t port);

    ~Acceptor() = default;

    Acceptor(const Acceptor&) = delete;
    Acceptor& operator=(const Acceptor&) = delete;

    void setNewConnectionCallback(NewConnectionCallback cb) {
        newConnectionCallback_ = std::move(cb);
    }

    void listen() const;

    void handleRead() const;
private:
    Socket listenSocket_{};
    InetAddress acceptAddress_{};
    NewConnectionCallback newConnectionCallback_{};
};

#endif //ACCEPTOR_H
