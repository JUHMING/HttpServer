//
// Created by py_01 on 26-7-23.
//

#ifndef MESSAGECODEC_H
#define MESSAGECODEC_H

#include<cstdint>
#include<cstddef>
#include<string>
#include <system_error>

class Buffer;

class MessageCodec {
public:
    enum class DecodeStatus {
        kSuccess,
        kNeedMoreData,
        kInvalidLength
    };

    static constexpr size_t kHeaderLength = sizeof(uint32_t);
    static constexpr size_t kDefaultMaxMessageLength = 64 * 1024 * 1024;

    explicit MessageCodec(const int maxMessageLength = kDefaultMaxMessageLength): maxMessageLength_(maxMessageLength) {}

    [[nodiscard]] std::string encode(const std::string& message) const;
    DecodeStatus tryDecode(Buffer* buffer, std::string& message) const;

    [[nodiscard]] size_t getMaxMessageLength() const {
        return maxMessageLength_;
    }

private:
    size_t maxMessageLength_;
};

#endif //MESSAGECODEC_H