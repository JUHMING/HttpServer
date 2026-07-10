//
// Created by py_01 on 26-6-30.
//

#ifndef CHANNEL_H
#define CHANNEL_H

#include<functional>
#include<cstdint>
#include <memory>

class EventLoop;

class Channel {
public:
    using EventCallback = std::function<void()>;

    Channel( const int fd, EventLoop *loop): fd_(fd), loop_(loop) {};
    ~Channel() = default;

    Channel(const Channel&) = delete;
    Channel& operator=(const Channel&) = delete;

    void tie(const std::shared_ptr<void>& obj);

    void remove();

    void handleEvent() const;

    void setReadCallback(EventCallback cb) { readCallback_ = std::move(cb); }
    void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
    void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb);}
    void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

    void enableReading() {events_ |= kReadEvent; update();};
    void enableWriting() {events_ |= kWriteEvent; update();};
    void disableReading() {events_ &= ~kReadEvent; update();};
    void disableWriting() {events_ &= ~kWriteEvent; update();};
    void disableAll() {events_ = kNoneEvent;};

    [[nodiscard]] bool isNoneEvent() const { return events_ == kNoneEvent; }
    [[nodiscard]] bool isReadEvent() const { return events_ & kReadEvent; }
    [[nodiscard]] bool isWriteEvent() const { return events_ & kWriteEvent; }

    void setRevents(const uint32_t revents) {revents_ = revents;}
    void setIndex(const int index) { index_ = index; }

    [[nodiscard]] uint32_t revents() const { return revents_; }
    [[nodiscard]] int index() const { return index_; }
    [[nodiscard]] int fd() const { return fd_; }
    [[nodiscard]] uint32_t events() const { return events_; }

    [[nodiscard]] EventLoop *loop() const { return loop_; }
private:
    void update();

    const int fd_{-1};
    EventLoop* loop_{nullptr};

    static const uint32_t kNoneEvent;
    static const uint32_t kReadEvent;
    static const uint32_t kWriteEvent;

    uint32_t events_{};
    uint32_t revents_{};
    int index_{};

    EventCallback readCallback_{};
    EventCallback writeCallback_{};
    EventCallback closeCallback_{};
    EventCallback errorCallback_{};

    std::weak_ptr<void> tie_;
    bool tied_{false};
};

#endif //CHANNEL_H
