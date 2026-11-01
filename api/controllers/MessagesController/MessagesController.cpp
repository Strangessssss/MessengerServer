#include "MessagesController.h"

#include <optional>

#include "../../http/HttpResponses/HttpResponses.h"
#include "../../http/JsonBody/JsonBody.h"
#include "../../json/JsonMapper/JsonMapper.h"

MessagesController::MessagesController(MessagesService& messagesService, const AuthGuard& authGuard)
    : messagesService(messagesService), authGuard(authGuard) {}

void MessagesController::registerRoutes(crow::SimpleApp& app) {
    CROW_ROUTE(app, "/chats/<string>/messages").methods(crow::HTTPMethod::POST)([this](const crow::request& request, const std::string& chatId) {
        return authGuard.withUser(request, [&](int userId) {
            return sendToChat(request, chatId, userId);
        });
    });
}

crow::response MessagesController::sendToChat(const crow::request& request, const std::string& chatId, int userId) {
    const std::optional<JsonBody> body = JsonBody::parse(request);
    const std::optional<std::string> text = body ? body->getString("text") : std::nullopt;
    if (!text.has_value()) {
        return HttpResponses::error(400, "text is required");
    }

    const MessageResponse response = messagesService.sendMessage(chatId, userId, *text);
    if (response.not_found()) {
        return HttpResponses::error(404, "Chat not found");
    }
    if (!response.success) {
        return HttpResponses::error(400, "Message was rejected");
    }
    return HttpResponses::json(201, JsonMapper::toJson(*response.response_data));
}
