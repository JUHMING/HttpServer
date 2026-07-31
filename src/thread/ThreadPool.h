//
// Created by py_01 on 26-7-15.
//

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include"TaskQueue.h"
#include<atomic>
#include<cstddef>
#include<functional>
#include<thread>
#include<vector>

class ThreadPool {
public:
    using Task = std::function<void()>;

    explicit ThreadPool(size_t threadNum = 0);
    ~ThreadPool();

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    void setThreadNum(size_t threadNum);
    void start();
    void stop();
    void run(Task task);

    [[nodiscard]] bool started() const { return running_; }
    [[nodiscard]] size_t threadNum() const { return threadNum_; }

private:
    void runInThread();

    size_t threadNum_{0};
    std::atomic_bool running_{false};
    TaskQueue queue_;
    std::vector<std::thread> threads_;
};

#endif //THREADPOOL_H
