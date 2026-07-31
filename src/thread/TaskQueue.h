//
// Created by py_01 on 26-7-15.
//

#ifndef TASKQUEUE_H
#define TASKQUEUE_H

#include<condition_variable>
#include<functional>
#include<mutex>
#include<queue>

class TaskQueue {
public:
    using Task = std::function<void()>;

    TaskQueue() = default;
    ~TaskQueue() = default;

    TaskQueue(const TaskQueue&) = delete;
    TaskQueue& operator=(const TaskQueue&) = delete;

    void push(Task task);
    bool pop(Task& task);
    void stop();
    [[nodiscard]] bool empty() const;

private:
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    std::queue<Task> queue_;
    bool stopped_{false};
};

#endif //TASKQUEUE_H
