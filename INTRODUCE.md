# NebulaServer 项目架构设计（第一版）

## 项目目标

最终实现一个高性能 C++ Web Server。

最终效果：

```text
浏览器访问：

http://127.0.0.1:8080
```

服务器能够：

```text
解析 HTTP 请求
返回静态页面
用户登录注册
连接数据库
支持高并发
日志记录
线程池处理任务
epoll事件驱动
```

技术目标参考：

- NGINX（高性能 Web Server）
- Muduo（C++ 网络库架构思想）

---

# 第一原则：拒绝 main.cpp 写到底

错误做法：

```text
main.cpp 3000行
```

正确做法：

```text
模块化 + 分层设计
```

---

# 项目整体目录结构

```text
NebulaServer/

├── CMakeLists.txt
│
├── src/                     # 所有实现文件
│   │
│   ├── main.cpp
│   │
│   ├── server/             # 服务器核心
│   │   ├── TcpServer.cpp
│   │   ├── TcpServer.h
│   │   ├── Acceptor.cpp
│   │   └── Acceptor.h
│   │
│   ├── net/                # 网络层
│   │   ├── Socket.cpp
│   │   ├── Socket.h
│   │   ├── InetAddress.cpp
│   │   ├── InetAddress.h
│   │   ├── Buffer.cpp
│   │   └── Buffer.h
│   │
│   ├── event/              # IO多路复用层
│   │   ├── EventLoop.cpp
│   │   ├── EventLoop.h
│   │   ├── Channel.cpp
│   │   ├── Channel.h
│   │   ├── Poller.cpp
│   │   ├── Poller.h
│   │   ├── EpollPoller.cpp
│   │   └── EpollPoller.h
│   │
│   ├── http/              # HTTP协议层
│   │   ├── HttpRequest.cpp
│   │   ├── HttpRequest.h
│   │   ├── HttpResponse.cpp
│   │   ├── HttpResponse.h
│   │   ├── HttpParser.cpp
│   │   └── HttpParser.h
│   │
│   ├── connection/         # TCP连接管理
│   │   ├── TcpConnection.cpp
│   │   └── TcpConnection.h
│   │
│   ├── thread/             # 线程池
│   │   ├── ThreadPool.cpp
│   │   ├── ThreadPool.h
│   │   ├── TaskQueue.cpp
│   │   └── TaskQueue.h
│   │
│   ├── database/           # 数据库模块
│   │   ├── ConnectionPool.cpp
│   │   ├── ConnectionPool.h
│   │   ├── MysqlConnection.cpp
│   │   └── MysqlConnection.h
│   │
│   ├── log/               # 日志模块
│   │   ├── Logger.cpp
│   │   ├── Logger.h
│   │   ├── LogFile.cpp
│   │   └── LogFile.h
│   │
│   └── utils/             # 工具类
│       ├── NonCopyable.h
│       ├── Timestamp.cpp
│       └── Timestamp.h
│
├── tests/                 # 单元测试
│   ├── test_socket.cpp
│   ├── test_buffer.cpp
│   └── test_threadpool.cpp
│
├── static/               # 静态网页资源
│   ├── index.html
│   ├── login.html
│   └── css/
│
├── config/
│   └── server.conf
│
├── logs/
│   └── server.log
│
└── README.md
```

---

# 模块说明

---

## 1. main.cpp

程序入口。

职责：

```text
创建服务器对象
启动服务器
```

最终代码应该类似：

```cpp
int main()
{
    TcpServer server;
    server.start();
}
```

原则：

```text
main.cpp 不允许写业务逻辑
```

---

## 2. net/ （网络基础层）

作用：

封装 Linux Socket API。

当前你写的是：

```cpp
socket()
bind()
listen()
close()
```

未来应该变成：

```cpp
Socket socket;
socket.bind();
socket.listen();
```

### Socket 类设计

```cpp
class Socket
{
public:
    Socket();

    void bind();

    void listen();

    int accept();

    void close();

private:
    int fd_;
};
```

设计原则：

```text
高层代码不能直接操作 Linux API
```

即：

```text
业务层不要碰 socket() bind() listen()
```

---

## 3. server/ （服务器核心）

负责整个服务器生命周期管理。

核心类：

```text
TcpServer
```

职责：

```text
启动服务器
管理连接
协调各模块
```

示例：

```cpp
class TcpServer
{
public:
    void start();
};
```

---

## 4. connection/ （连接管理）

作用：

管理每一个客户端连接。

模型：

```text
Client A → TcpConnection object

Client B → TcpConnection object

Client C → TcpConnection object
```

类设计：

```cpp
class TcpConnection
{
private:
    int fd_;
};
```

职责：

```text
管理单个客户端生命周期
```

---

## 5. event/ （事件驱动层，核心模块）

决定服务器性能。

基于：

```text
epoll
```

包含：

### EventLoop

负责：

```text
等待事件
处理事件
循环执行
```

逻辑：

```cpp
while(true)
{
    epoll_wait();

    handle_events();
}
```

---

### Channel

作用：

```text
封装 fd + 监听事件
```

例如：

```text
socket fd = 5

监听：

可读事件
```

---

### Poller

统一 IO 多路复用接口。

支持：

```text
select
poll
epoll
```

设计目标：

```text
面向抽象编程
```

---

## 6. http/ （HTTP协议层）

负责解析浏览器请求。

例如：

```http
GET /index.html HTTP/1.1
Host: localhost
```

需要解析：

```text
请求方法
请求路径
请求头
```

---

### HttpRequest

保存请求信息。

```cpp
method = GET

path = /index.html
```

---

### HttpResponse

生成响应。

例如：

```http
HTTP/1.1 200 OK

<html>Hello</html>
```

---

## 7. thread/ （线程池）

目的：

处理高并发。

错误模型：

```text
一个客户端 = 一个线程
```

正确模型：

```text
固定线程池

例如：

8 worker threads
```

模块：

```text
ThreadPool
TaskQueue
```

---

## 8. database/ （数据库模块）

功能：

用户登录 / 注册。

例如：

```sql
SELECT * FROM users
WHERE name='admin'
```

数据库支持：

```text
MySQL
SQLite
```

模块：

```text
ConnectionPool
MysqlConnection
```

---

## 9. log/ （日志系统）

记录：

```text
客户端连接

错误信息

请求内容

服务器状态
```

日志示例：

```text
[INFO] Client connected

[ERROR] Bind failed
```

---

# 当前阶段只实现哪些模块？

不要全部实现。

第一阶段只需要：

```text
NebulaServer/

src/

├── main.cpp

├── net/
│   ├── Socket.h
│   └── Socket.cpp

├── server/
│   ├── TcpServer.h
│   └── TcpServer.cpp
```

---

# 当前任务（Task 3）

开始工程化重构。

不要继续改当前 server.cpp。

把代码拆分为：

```text
Socket 类

TcpServer 类

main.cpp
```

目标：

```cpp
int main()
{
    TcpServer server;

    server.start();
}
```

---

# 当前阶段转变

你正在从：

```text
会写 demo
```

进入：

```text
会设计工程项目
```

这是一次质变。

---

# 当前原则（必须遵守）

```text
不要继续写过程式 socket 代码

开始面向对象封装
```

---

# 当前开发路线

```text
阶段1：

Socket封装
TcpServer封装

↓

阶段2：

非阻塞 IO

↓

阶段3：

epoll

↓

阶段4：

EventLoop + Reactor

↓

阶段5：

HTTP协议解析

↓

阶段6：

线程池

↓

阶段7：

数据库

↓

阶段8：

完整 Web Server
```

---

# 一句话总结

```text
停止继续写 demo

开始工程化重构

从过程式编程进入面向对象架构设计
```