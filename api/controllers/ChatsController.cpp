#include "ChatsController.h"

#include <optional>
#include <vector>

#include "../http/HttpResponses.h"
#include "../http/JsonBody.h"
#include "../json/JsonMapper.h"

ChatsController::ChatsController(ChatsService& chatsService, const AuthGuard& authGuard)
    : chatsService(chatsService), authGuard(authGuard) {}

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
    CROW_ROUTE(app, "/chats/<string>").methods(crow::HTTPMethod::DELETE)([this](const crow::request& request, const std::string& chatId) {
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
    std::optional<std::vector<int>> userIds = body ? body->getIntList("userIds") : std::nullopt;
    if (!userIds.has_value()) {
        return HttpResponses::error(400, "userIds must be a list of user IDs");
    }

    userIds->push_back(userId);
    const ChatResponse response = chatsService.addChat(*userIds);
    if (!response.success_) {
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
