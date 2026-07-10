//
// Created by py_01 on 26-6-18.
//

#ifndef SOCKET_H
#define SOCKET_H

#include<utility>
class InetAddress;

class Socket {
public:
    Socket();
    ~Socket();

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    void bind(const InetAddress& addr) const;

    void listen(int backlog = 5) const;

    [[nodiscard]] std::pair<int, InetAddress> accept() const;

    [[nodiscard]] int fd() const;
private:
    int fd_;
};

#endif //SOCKET_H
