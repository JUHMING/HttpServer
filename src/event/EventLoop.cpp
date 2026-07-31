//
// Created by py_01 on 26-7-9.
//

#include"EventLoop.h"
#include"EpollPoller.h"
#include"Channel.h"
#include<stdexcept>
#include<sys/eventfd.h>
#include<unistd.h>
#include<cstdint>
#include<cstring>
#include<cerrno>
#include<iostream>
#include<utility>

EventLoop::EventLoop() : looping_(false),
                        poller_(std::make_unique<EpollPoller>(this)),
                        threadId_(std::this_thread::get_id())
{
    wakeupFd_ = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (wakeupFd_ < 0) {
        throw std::runtime_error("eventfd() failed in EventLoop::EventLoop");
    }
    wakeupChannel_ = std::make_unique<Channel>(wakeupFd_, this);
    wakeupChannel_->setReadCallback([this] { handleWakeup(); });
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() {
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    if (wakeupFd_ >= 0) {
        ::close(wakeupFd_);
    }
}

void EventLoop::loop(const bool flag) {
    looping_ = flag;
    quit_ = false;

    while(!quit_) {
        activeChannels_.clear();
        poller_->poll(waitTime_, &activeChannels_);
        for(auto channel : activeChannels_) {
            channel->handleEventWithGuard();
        }
        doPendingFunctors();
    }
    looping_ = false;
}
void EventLoop::quit() {
    quit_ = true;
    if (!isInLoopThread()) {
        wakeup();
    }
}

void EventLoop::runInLoop(Functor cb) {
    if (isInLoopThread()) {
        cb();
    } else {
        queueInLoop(std::move(cb));
    }
}

void EventLoop::queueInLoop(Functor cb) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingFunctors_.push_back(std::move(cb));
    }

    if (!isInLoopThread() || callingPendingFunctors_) {
        wakeup();
    }
}

bool EventLoop::isInLoopThread() const {
    return threadId_ == std::this_thread::get_id();
}

void EventLoop::updateChannel(Channel *channel) const {
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel) const {
    poller_->removeChannel(channel);
}

void EventLoop::wakeup() const {
    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one) && errno != EAGAIN) {
        std::cerr << "[ERROR]: EventLoop::wakeup write failed: " << std::strerror(errno) << std::endl;
    }
}

void EventLoop::handleWakeup() const {
    uint64_t one = 1;
    ssize_t n = ::read(wakeupFd_, &one, sizeof(one));
    if (n < 0 && errno != EAGAIN) {
        std::cerr << "[ERROR]: EventLoop::handleWakeup read failed: " << std::strerror(errno) << std::endl;
    }
}

void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for (auto& functor : functors) {
        functor();
    }
    callingPendingFunctors_ = false;
}




