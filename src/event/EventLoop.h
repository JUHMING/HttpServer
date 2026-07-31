//
// Created by py_01 on 26-7-9.
//

#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include<memory>
#include<vector>
#include <functional>
#include <vector>
#include <mutex>
#include <thread>

class Channel;
class Poller;

class EventLoop {
public:
    using Functor = std::function<void()>;
    EventLoop();
    ~EventLoop();

    EventLoop(const EventLoop &) = delete;
    EventLoop &operator=(const EventLoop &) = delete;

    void loop(bool flag = true);
    void quit();

    void runInLoop(Functor cb);
    void queueInLoop(Functor cb);

    [[nodiscard]] bool isInLoopThread() const;

    void updateChannel(Channel* channel) const;
    void removeChannel(Channel* channel) const;

private:
    void wakeup() const;
    void handleWakeup() const;
    void doPendingFunctors();

    std::atomic_bool looping_{false};
    std::atomic_bool quit_{false};
    std::unique_ptr<Poller> poller_;
    using ChannelList = std::vector<Channel*>;
    ChannelList activeChannels_;
    int waitTime_{10000};

    const std::thread::id threadId_;
    int wakeupFd_{-1};
    std::unique_ptr<Channel> wakeupChannel_;

    std::mutex mutex_;
    std::vector<Functor> pendingFunctors_;
    bool callingPendingFunctors_{false};
};

#endif //EVENTLOOP_H
