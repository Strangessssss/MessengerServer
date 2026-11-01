#include "ChatsController.h"

#include <optional>
#include <vector>

#include "../../http/HttpResponses/HttpResponses.h"
#include "../../http/JsonBody/JsonBody.h"
#include "../../json/JsonMapper/JsonMapper.h"

ChatsController::ChatsController(ChatsService& chatsService, UsersService& usersService, const AuthGuard& authGuard)
    : chatsService(chatsService), usersService(usersService), authGuard(authGuard) {}

void ChatsController::registerRoutes(crow::SimpleApp& app) {
    CROW_ROUTE(app, "/chats").methods(crow::HTTPMethod::GET)([this](const crow::request& request) {
        return authGuard.withUser(request, [&](int userId) {
            return getChats(userId);
        });
    });
    CROW_ROUTE(app, "/chats").methods(crow::HTTPMethod::POST)([this](const crow::request& request) {
        return authGuard.withUser(request, [&](int userId) {
            return createChat(request, userId);
        });
    });
    CROW_ROUTE(app, "/chats/<string>").methods(crow::HTTPMethod::GET)([this](const crow::request& request, const std::string& chatId) {
        return authGuard.withUser(request, [&](int userId) {
            return getChatView(chatId, userId);
        });
    });
    CROW_ROUTE(app, "/chats/<string>").methods(crow::HTTPMethod::Delete)([this](const crow::request& request, const std::string& chatId) {
        return authGuard.withUser(request, [&](int userId) {
            return deleteChat(chatId, userId);
        });
    });
}

crow::response ChatsController::getChats(int userId) {
    const ChatSummaryListResponse response = chatsService.getChats(userId);
    if (response.not_found()) {
        return HttpResponses::json(200, JsonMapper::toJsonList(std::vector<ChatSummary>{}));
    }
    return HttpResponses::json(200, JsonMapper::toJsonList(*response.response_data));
}

crow::response ChatsController::createChat(const crow::request& request, int userId) {
    const std::optional<JsonBody> body = JsonBody::parse(request);
    std::optional<std::string> username = body ? body->getString("username") : std::nullopt;
    if (!username.has_value()) {
        return HttpResponses::error(400, "Username missing");
    }

    const UserIdResponse partnerUserId = usersService.getUserIdByUsername(username.value());
    if (partnerUserId.not_found()) {
        return HttpResponses::error(404, "User not found");
    }
    if (*partnerUserId.response_data == userId) {
        return HttpResponses::error(400, "You can't create a chat with yourself");
    }

    const ChatResponse response = chatsService.addChat({userId, *partnerUserId.response_data});
    if (!response.success) {
        return HttpResponses::error(400, "A chat needs at least one other existing user");
    }
    return HttpResponses::json(200, JsonMapper::toJson(*response.response_data));
}

crow::response ChatsController::getChatView(const std::string& chatId, int userId) {
    const ChatViewResponse response = chatsService.getChatView(chatId, userId);
    if (response.not_found()) {
        return HttpResponses::error(404, "Chat not found");
    }
    return HttpResponses::json(200, JsonMapper::toJson(*response.response_data));
}

crow::response ChatsController::deleteChat(const std::string& chatId, int userId) {
    const ChatResponse response = chatsService.deleteChat(chatId, userId);
    if (response.not_found()) {
        return HttpResponses::error(404, "Chat not found");
    }
    return crow::response(204);
}
