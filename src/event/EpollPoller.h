//
// Created by py_01 on 26-7-9.
//

#ifndef EPOLLPOLLER_H
#define EPOLLPOLLER_H

#include"Poller.h"
#include<sys/epoll.h>
#include<vector>

class EpollPoller : public Poller {
public:
    explicit EpollPoller(EventLoop* loop);
    ~EpollPoller() override;

    void poll(int timeoutMs, ChannelList* activeChannel) override;

    void updateChannel(Channel* channel) override;

    void removeChannel(Channel* channel) override;

private:
    void update(int operation, Channel* channel) const;

    static constexpr int kInitEventListSize = 16;

    using EventList = std::vector<struct epoll_event>;
    EventList events_;
    int epollFd_{-1};
};

#endif //EPOLLPOLLER_H
