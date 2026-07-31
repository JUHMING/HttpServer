//
// Created by py_01 on 26-7-15.
//

#include"TaskQueue.h"
#include<utility>

void TaskQueue::push(Task task) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (stopped_) {
            return;
        }
        queue_.push(std::move(task));
    }
    condition_.notify_one();
}

bool TaskQueue::pop(Task& task) {
    std::unique_lock<std::mutex> lock(mutex_);
    condition_.wait(lock, [this] {
        return stopped_ || !queue_.empty();
    });

    if (queue_.empty()) {
        return false;
    }

    task = std::move(queue_.front());
    queue_.pop();
    return true;
}

void TaskQueue::stop() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stopped_ = true;
    }
    condition_.notify_all();
}

bool TaskQueue::empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
}
