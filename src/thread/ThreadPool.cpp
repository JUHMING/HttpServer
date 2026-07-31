//
// Created by py_01 on 26-7-15.
//

#include"ThreadPool.h"
#include<exception>
#include<iostream>
#include<utility>

ThreadPool::ThreadPool(size_t threadNum) : threadNum_(threadNum) {}

ThreadPool::~ThreadPool() {
    stop();
}

void ThreadPool::setThreadNum(size_t threadNum) {
    if (running_) {
        return;
    }
    threadNum_ = threadNum;
}

void ThreadPool::start() {
    if (running_) {
        return;
    }

    running_ = true;
    threads_.reserve(threadNum_);
    for (size_t i = 0; i < threadNum_; ++i) {
        threads_.emplace_back([this] {
            runInThread();
        });
    }
}

void ThreadPool::stop() {
    if (!running_) {
        return;
    }

    running_ = false;
    queue_.stop();
    for (auto& thread : threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    threads_.clear();
}

void ThreadPool::run(Task task) {
    if (!task) {
        return;
    }

    if (!running_ || threadNum_ == 0) {
        task();
        return;
    }

    queue_.push(std::move(task));
}

void ThreadPool::runInThread() {
    while (running_) {
        Task task;
        if (!queue_.pop(task)) {
            break;
        }

        try {
            task();
        } catch (const std::exception& ex) {
            std::cerr << "[ERROR]: ThreadPool task exception: " << ex.what() << std::endl;
        } catch (...) {
            std::cerr << "[ERROR]: ThreadPool task unknown exception" << std::endl;
        }
    }
}
