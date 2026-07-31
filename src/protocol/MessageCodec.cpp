//
// Created by py_01 on 26-7-24.
//

#include "MessageCodec.h"
#include"net/Buffer.h"

#include<arpa/inet.h>
#include<cstring>
#include<limits>
#include<stdexcept>

std::string MessageCodec::encode(const std::string& message) const {
    if (message.length() > maxMessageLength_) {
        throw std::length_error("MessageCodec::encode message length exceeds maxMessageLength");
    }

    uint32_t length = htonl(static_cast<uint32_t>(message.size()));
    std::string encodedMessage;
    encodedMessage.resize(kHeaderLength + message.size());
    std::memcpy(encodedMessage.data(), &length, kHeaderLength);
    std::memcpy(encodedMessage.data() + kHeaderLength, message.data(), message.size());
    return encodedMessage;
}

MessageCodec::DecodeStatus MessageCodec::tryDecode(Buffer* buffer, std::string& encodedMessage) const {
    if (buffer == nullptr) {
        return DecodeStatus::kInvalidLength;
    }

    if (buffer->readableBytes() < kHeaderLength) {
        return DecodeStatus::kNeedMoreData;
    }

    uint32_t networkLength = 0;
    std::memcpy(&networkLength, buffer->peek(), kHeaderLength);
    const uint32_t length = ntohl(networkLength);

    if (length > maxMessageLength_) {
        return DecodeStatus::kInvalidLength;
    }

    if (buffer->readableBytes() < kHeaderLength + length) {
        return DecodeStatus::kNeedMoreData;
    }
    buffer->retrieve(kHeaderLength);
    encodedMessage = buffer->retrieveString(length);
    return DecodeStatus::kSuccess;
}