#include "event/EventLoop.h"
#include "net/InetAddress.h"
#include "server/TcpServer.h"
#include <iostream>

int main() {
    std::cout << "[INFO]: NebulaServer Core Engine starting up..." << std::endl;

    // 1. 创建事件循环核心（大管家）
    EventLoop loop;

    // 2. 设定服务器要监听的专属招牌（8080 端口）
    InetAddress listenAddr(8080);

    // 3. 拉起最高总指挥部
    TcpServer server(&loop, listenAddr, "NebulaServer-Core");

    // 4. 总指挥部开门营业
    server.start();

    std::cout << "[INFO]: Reactor Engine initialized successfully. Entering loop..." << std::endl;

    // 5. 让心脏开始跳动！主线程在此处挂起睡眠，静待网络中断信号
    loop.loop();

    return 0;
}