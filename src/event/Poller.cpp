//
// Created by py_01 on 26-7-2.
//
#include"Poller.h"
#include "Channel.h"

bool Poller::hasChannel(Channel *channel) const {
    auto it = channels_.find(channel->fd());
    return it != channels_.end() && it->second == channel;
}
