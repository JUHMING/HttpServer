//
// Created by py_01 on 26-7-9.
//

#include"EventLoop.h"
#include"EpollPoller.h"
#include"Channel.h"
#include<stdexcept>

EventLoop::EventLoop() : looping_(false),
                        poller_(std::make_unique<EpollPoller>(this)){}

EventLoop::~EventLoop() = default;

void EventLoop::loop(const bool flag) {
    looping_ = flag;

    while(looping_) {
        activeChannels_.clear();
        poller_->poll(waitTime_, &activeChannels_);
        for(auto channel : activeChannels_) {
            channel->handleEvent();
        }
    }
    looping_ = false;
}

void EventLoop::updateChannel(Channel *channel) const {
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel) const {
    poller_->removeChannel(channel);
}



