#pragma once
#include <string>
#include <vector>

#include "../../../../common/response/Response.h"
#include "../../../../entities/Message.h"

struct ChatView {
    std::string chatId;
    std::string partnerUsername;
    std::vector<Message> messages;
};

struct ChatViewResponse: Response<ChatView> {
    using Response::Response;
};
