#pragma once
#include <crow.h>

#include <vector>

#include "../../entities/Chat.h"
#include "../../entities/Message.h"
#include "../../services/ChatsService/DTOs/responses/ChatSummaryListResponse.h"
#include "../../services/ChatsService/DTOs/responses/ChatViewResponse.h"
#include "../../services/AuthService/DTOs/responses/AuthResponse.h"

// Timestamps are milliseconds since the Unix epoch.
namespace JsonMapper {
    crow::json::wvalue toJson(const AuthResult& auth);
    crow::json::wvalue toJson(const Chat& chat);
    crow::json::wvalue toJson(const ChatSummary& summary);
    crow::json::wvalue toJson(const ChatView& view);
    crow::json::wvalue toJson(const Message& message);

    template <typename T>
    crow::json::wvalue toJsonList(const std::vector<T>& items) {
        std::vector<crow::json::wvalue> list;
        for (const T& item : items) {
            list.push_back(toJson(item));
        }
        return crow::json::wvalue(std::move(list));
    }
}
