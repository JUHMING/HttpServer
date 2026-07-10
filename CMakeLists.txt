cmake_minimum_required(VERSION 3.22)
project(NebulaServer LANGUAGES CXX) # 明确声明项目名称与语言

# 严格开启 C++20 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ==========================================
# 1. 编译核心网络与事件驱动静态库 (network)
# ==========================================
add_library(network STATIC
        # net/ 网络层
        src/net/Socket.cpp
        src/net/InetAddress.cpp

        # server/ 核心层
        src/server/Acceptor.cpp

        # event/ 事件驱动层 (提前把未来几天要写的战壕挖好)
        src/event/Channel.cpp
        src/event/Poller.h
        src/event/Poller.cpp
        src/event/EpollPoller.h
        src/event/EpollPoller.cpp
        src/event/EventLoop.h
        src/event/EventLoop.cpp
        src/connection/TcpConnection.h
        src/connection/TcpConnection.cpp
        src/server/TcpServer.h
        src/server/TcpServer.cpp
        #  src/event/Poller.cpp
       # src/event/EpollPoller.cpp
       # src/event/EventLoop.cpp
)

# 现代 CMake 核心：指定头文件搜索根目录为 src/
# 这样我们在代码里就可以优雅地写 #include "net/Socket.h" 或 "event/Channel.h"
target_include_directories(network PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ==========================================
# 2. 编译主程序目标 (server)
# ==========================================
add_executable(server
        src/main.cpp
)

# 链接核心库
target_link_libraries(server PRIVATE
        network
)