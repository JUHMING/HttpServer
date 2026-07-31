//
// Created by py_01 on 26-7-9.
//

#include"EpollPoller.h"
#include"Channel.h"
#include<unistd.h>
#include<cstring>
#include <iostream>
#include<stdexcept>
#include<cerrno>
#include <system_error>

EpollPoller::EpollPoller(EventLoop* loop) : Poller(loop) {
    epollFd_ = ::epoll_create1(EPOLL_CLOEXEC);
    events_ = EventList(kInitEventListSize);
    if (epollFd_ < 0) {
        throw std::runtime_error("epoll_create1() failed in EpollPoller::EpollPoller");
    }
}

EpollPoller::~EpollPoller() {
    if (epollFd_ >= 0) {
        ::close(epollFd_);
    }
}

void EpollPoller::poll(const int timeoutMs, ChannelList* activeChannels) {
    int numEvents = epoll_wait(epollFd_, events_.data(), static_cast<int>(events_.size()), timeoutMs);
    const int savedErrno = errno;
    if (numEvents < 0) {
        if (savedErrno == EINTR) {
            return;
        }
        throw std::system_error(savedErrno, std::generic_category(), "epoll_wait() failed in EpollPoller::poll");
    }

    if (numEvents == 0) {
        //timeout is a normal occurrence
        return;
    }

    for (int i = 0; i < numEvents; ++i) {
        auto* channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->setRevents(events_[i].events);
        activeChannels->push_back(channel);
    }

    if (static_cast<size_t>(numEvents) == events_.size()) {
        events_.resize(events_.size() * 2);
    }
}

void EpollPoller::updateChannel(Channel* channel) {
    const int state = channel->index();

    if (state == Channel::State::kNew || state == Channel::State::kDeleted) {
        int fd = channel->fd();
        if (state == Channel::State::kNew) {
            channels_[fd] = channel;
        }
        channel->setIndex(Channel::State::kAdded);
        update(EPOLL_CTL_ADD, channel);
    }else {
        if (channel->isNoneEvent()) {
            update(EPOLL_CTL_DEL, channel);
            channel->setIndex(Channel::State::kDeleted);
        }else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EpollPoller::removeChannel(Channel* channel) {
    const int fd = channel->fd();
    channels_.erase(fd);

    const int state = channel->index();
    if (state == Channel::State::kAdded) {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->setIndex(Channel::State::kNew);
}

void EpollPoller::update(const int operation, Channel* channel) const {
    struct epoll_event event{};
    ::memset(&event, 0, sizeof(event));
    event.events = channel->events();
    event.data.ptr = channel;
    int fd = channel->fd();
    if (::epoll_ctl(epollFd_, operation, fd, &event) < 0) {
        throw std::runtime_error("epoll_ctl() failed in EpollPoller::update");
    }
}