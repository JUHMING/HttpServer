//
// Created by py_01 on 26-6-22.
//
#include"InetAddress.h"

InetAddress::InetAddress(const uint16_t port) {
    addr_.sin_family = IP_TYPE;
    addr_.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_.sin_port = htons(port);
}

InetAddress::InetAddress(const std::string_view ip, const uint16_t port) {
    addr_.sin_family = IP_TYPE;
    std::string tmp(ip);
    if (inet_pton(IP_TYPE, tmp.c_str(), &addr_.sin_addr) <= 0) {
        throw std::runtime_error("Invalid IP address");
    }
    addr_.sin_port = htons(port);
}

InetAddress::InetAddress(const sockaddr_in& addr) : addr_{addr} {}

const sockaddr_in& InetAddress::getAddr() const{
    return addr_;
}

const sockaddr* InetAddress::getSockAddr() const {
    return reinterpret_cast<const sockaddr*>(&addr_);
}

std::string InetAddress::getIp() const {
    char ip_str[INET_ADDRSTRLEN]{};
    if (auto ip = inet_ntop(IP_TYPE, &addr_.sin_addr, ip_str, sizeof(ip_str)); ip == nullptr) {
        throw std::runtime_error("inet_ntop failed to convert IP address");
    }
    return {ip_str};
}

uint16_t InetAddress::getPort() const {
    return ntohs(addr_.sin_port);
}

socklen_t InetAddress::getSize() const {
    return sizeof(addr_);
}

std::string InetAddress::toString() const {
    return std::string(getIp()) + ":" + std::to_string(getPort());
}
