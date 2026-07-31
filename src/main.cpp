#include "server/TcpServer.h"
#include "event/EventLoop.h"
#include "net/InetAddress.h"
#include <iostream>
#include <csignal>
#include <thread>
#include <mutex>

int main()
{
    ::signal(SIGPIPE, SIG_IGN);

    try
    {
        EventLoop loop;

        InetAddress listenAddr("0.0.0.0", 8880);

        TcpServer server(&loop, listenAddr, "HTTPServer");


        TcpConnection::TcpConnectionPtr currentConn;
        std::mutex connMutex;


        // 连接回调
        server.setConnectionCallback(
            [&](const TcpConnection::TcpConnectionPtr& conn)
            {
                std::lock_guard<std::mutex> lock(connMutex);

                if (conn->isConnected())
                {
                    currentConn = conn;

                    std::cout
                        << "[INFO] New connection: "
                        << conn->getName()
                        << std::endl;
                }
                else
                {
                    std::cout
                        << "[INFO] Connection closed: "
                        << conn->getName()
                        << std::endl;

                    if(currentConn == conn)
                    {
                        currentConn.reset();
                    }
                }
            }
        );


        // 收消息
        server.setMessageCallback(
            [](const TcpConnection::TcpConnectionPtr& conn,
               const std::string& message)
            {
                std::cout
                    << "[CLIENT] "
                    << message
                    << std::endl;

                // echo
                conn->send(message);
            }
        );


        server.start();


        std::cout
            << "=================================\n"
            << " TCP Server started\n"
            << " Listen: 0.0.0.0:8880\n"
            << "================================="
            << std::endl;


        /*
         * 服务器交互窗口
         */
        std::thread consoleThread(
            [&]()
            {
                std::string message;

                while(std::getline(std::cin, message))
                {
                    std::lock_guard<std::mutex> lock(connMutex);

                    if(currentConn)
                    {
                        currentConn->send(message);

                        std::cout
                            << "[SERVER] "
                            << message
                            << std::endl;
                    }
                    else
                    {
                        std::cout
                            << "[WARN] No client connected"
                            << std::endl;
                    }
                }
            }
        );


        loop.loop();


        consoleThread.join();
    }
    catch (const std::exception& e)
    {
        std::cerr
            << "[FATAL] "
            << e.what()
            << std::endl;

        return -1;
    }

    return 0;
}