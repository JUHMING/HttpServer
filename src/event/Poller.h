//
// Created by py_01 on 26-7-2.
//

#ifndef POLLER_H
#define POLLER_H

#include<unordered_map>
#include<vector>

class Channel;
class EventLoop;

class Poller {
public:
    using ChannelList = std::vector<Channel*>;
    explicit Poller(EventLoop* loop): loop_(loop) {};
    virtual ~Poller() = default;

    Poller(const Poller&) = delete;
    Poller& operator=(const Poller&) = delete;

    virtual void poll(int timeoutMs, ChannelList* activeChannel) = 0;

    virtual void updateChannel(Channel* channel) = 0;

    virtual void removeChannel(Channel* channel) = 0;

    virtual bool hasChannel(Channel* channel) const;

    EventLoop* ownerLoop() const { return loop_; }

protected:
    using ChannelMap = std::unordered_map<int, Channel*>;
    ChannelMap channels_;
private:
    EventLoop* loop_{nullptr};
};


#endif //POLLER_H
