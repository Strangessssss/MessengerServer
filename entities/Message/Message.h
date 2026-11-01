#pragma once
#include <chrono>
#include <string>

#include "../../enums/MessageStatus/MessageStatus.h"

struct Message {

    std::string id; // GUID
    int seq;

    std::string chatId;

    int sender;

    std::string text;

    MessageStatus status = Pending;

    std::chrono::system_clock::time_point createdAt;
};
