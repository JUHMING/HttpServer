//
// Created by py_01 on 26-6-22.
//

#ifndef INETADDRESS_H
#define INETADDRESS_H

#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdexcept>

#define IP_TYPE AF_INET

class InetAddress{
public:
    InetAddress() = default;

    explicit InetAddress(uint16_t port);
    InetAddress(std::string_view ip, uint16_t port);
    explicit InetAddress(const sockaddr_in& addr);

    ~InetAddress() = default;

    [[nodiscard]] const sockaddr_in& getAddr() const;

    [[nodiscard]] const sockaddr* getSockAddr() const;

    [[nodiscard]] socklen_t getSize() const;

    [[nodiscard]] std::string getIp() const;

    [[nodiscard]] uint16_t getPort() const;

    [[nodiscard]] std::string toString() const;
private:
    sockaddr_in addr_{};
};


#endif //INETADDRESS_H
