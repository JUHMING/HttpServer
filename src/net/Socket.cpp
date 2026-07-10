//
// Created by py_01 on 26-6-18.
//

#include"Socket.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdexcept>
#include"InetAddress.h"

Socket::Socket() : fd_(-1){
     fd_ = socket(IP_TYPE, SOCK_STREAM, 0);
     if (fd_ == -1) {
          throw std::runtime_error("socket creation failed");
     }
}

Socket::~Socket() {
     if (fd_ != -1) {
          ::close(fd_);
     }
}

void Socket::bind(const InetAddress& addr) const {
     if (fd_ == -1) {
          throw std::runtime_error("fd_ is -1, Socket bind failed");
     }

     if (::bind(fd_, addr.getSockAddr(), sizeof(sockaddr_in)) < 0) {
          throw std::runtime_error("Socket bind failed");
     }
}

void Socket::listen(const int backlog) const {
     if (fd_ == -1) {
          throw std::runtime_error("fd_ is -1, Socket listen failed");
     }
     if (::listen(fd_, backlog) < 0) {
          throw std::runtime_error("Socket listen failed");
     }
}

std::pair<int, InetAddress> Socket::accept() const {
     if (fd_ == -1) {
          std::cerr << "fd_ is -1, Socket accept failed" << std::endl;
          return {-1, InetAddress()};
     }
     struct sockaddr_in addr{};
     socklen_t len = sizeof(addr);
     int fd = ::accept(fd_, (struct sockaddr*)&addr, &len);

     if (fd == -1) {
          std::cerr << "Socket accept failed" << std::endl;
          return {-1, InetAddress()};
     }
     return {fd, InetAddress(addr)};
}

int Socket::fd() const {
     return fd_;
}

