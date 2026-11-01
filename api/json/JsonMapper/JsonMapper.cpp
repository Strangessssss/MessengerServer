#include "JsonMapper.h"

#include <chrono>
#include <cstdint>
#include <string>

namespace {
    std::int64_t toMilliseconds(std::chrono::system_clock::time_point time) {
        return std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch()).count();
    }

    std::string toString(MessageStatus status) {
        switch (status) {
            case Pending: return "pending";
            case Sent: return "sent";
            case Read: return "read";
            case Error: return "error";
        }
        return "unknown";
    }
}

crow::json::wvalue JsonMapper::toJson(const AuthResult& auth) {
    crow::json::wvalue json;
    json["userId"] = auth.userId;
    json["token"] = auth.token;
    return json;
}

crow::json::wvalue JsonMapper::toJson(const Chat& chat) {
    std::vector<crow::json::wvalue> userIds;
    for (const int userId : chat.userIds) {
        userIds.emplace_back(userId);
    }

    crow::json::wvalue json;
    json["id"] = chat.id;
    json["userIds"] = crow::json::wvalue(std::move(userIds));
    json["createdAt"] = toMilliseconds(chat.createdAt);
    return json;
}

crow::json::wvalue JsonMapper::toJson(const ChatSummary& summary) {
    crow::json::wvalue json;
    json["chatId"] = summary.chatId;
    json["partnerUsername"] = summary.partnerUsername;
    json["unreadCount"] = summary.unreadCount;
    if (summary.latestMessage.has_value()) {
        json["latestMessage"]["text"] = summary.latestMessage->text;
        json["latestMessage"]["createdAt"] = toMilliseconds(summary.latestMessage->createdAt);
        json["latestMessage"]["status"] = toString(summary.latestMessage->status);
    } else {
        json["latestMessage"] = nullptr;
    }
    return json;
}

crow::json::wvalue JsonMapper::toJson(const ChatView& view) {
    crow::json::wvalue json;
    json["chatId"] = view.chatId;
    json["partnerUsername"] = view.partnerUsername;
    json["messages"] = toJsonList(view.messages);
    return json;
}

crow::json::wvalue JsonMapper::toJson(const Message& message) {
    crow::json::wvalue json;
    json["id"] = message.id;
    json["seq"] = message.seq;
    json["chatId"] = message.chatId;
    json["senderId"] = message.sender;
    json["text"] = message.text;
    json["status"] = toString(message.status);
    json["createdAt"] = toMilliseconds(message.createdAt);
    return json;
}
