//
// Created by py_01 on 26-7-9.
//

#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include<memory>
#include<vector>

class Channel;
class Poller;

class EventLoop {
public:
    EventLoop();
    ~EventLoop();

    EventLoop(const EventLoop &) = delete;
    EventLoop &operator=(const EventLoop &) = delete;

    void loop(bool flag = true);

    void updateChannel(Channel* channel) const;
    void removeChannel(Channel* channel) const;

private:
    bool looping_{false};
    std::unique_ptr<Poller> poller_;
    using ChannelList = std::vector<Channel*>;
    ChannelList activeChannels_;
    int waitTime_{10000};
};

#endif //EVENTLOOP_H
