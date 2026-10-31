#pragma once
#include <chrono>
#include <optional>
#include <string>
#include <vector>

#include "../../../../common/response/Response.h"
#include "../../../../enums/MessageStatus.h"

struct LatestMessage {
    std::string text;
    std::chrono::system_clock::time_point createdAt;
    MessageStatus status = Pending;
};

struct ChatSummary {
    std::string chatId;
    std::string partnerUsername;
    std::optional<LatestMessage> latestMessage;
};

struct ChatSummaryListResponse: Response<std::vector<ChatSummary>> {
    using Response::Response;
};
