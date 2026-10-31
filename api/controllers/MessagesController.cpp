#include "MessagesController.h"

#include <charconv>
#include <cstring>
#include <optional>

#include "../http/HttpResponses.h"
#include "../http/JsonBody.h"
#include "../json/JsonMapper.h"

namespace {
    crow::response toHttpResponse(const MessageResponse& response) {
        if (response.not_found()) {
            return HttpResponses::error(404, "Chat or user not found");
        }
        if (!response.success) {
            return HttpResponses::error(400, "Message was rejected");
        }
        return HttpResponses::json(201, JsonMapper::toJson(*response.response_data));
    }

    // A missing parameter means from the start, and an invalid one reads as nothing.
    std::optional<int> readAfterSeq(const crow::request& request) {
        const char* value = request.url_params.get("afterSeq");
        if (value == nullptr) {
            return 0;
        }

        int afterSeq = 0;
        const char* end = value + std::strlen(value);
        const auto [parsedEnd, error] = std::from_chars(value, end, afterSeq);
        if (error != std::errc{} || parsedEnd != end) {
            return std::nullopt;
        }
        return afterSeq;
    }
}

MessagesController::MessagesController(MessagesService& messagesService, const AuthGuard& authGuard)
    : messagesService(messagesService), authGuard(authGuard) {}

void MessagesController::registerRoutes(crow::SimpleApp& app) {
    CROW_ROUTE(app, "/messages").methods(crow::HTTPMethod::POST)([this](const crow::request& request) {
        return authGuard.withUser(request, [&](int userId) {
            return sendToUser(request, userId);
        });
    });
    CROW_ROUTE(app, "/chats/<string>/messages").methods(crow::HTTPMethod::POST)([this](const crow::request& request, const std::string& chatId) {
        return authGuard.withUser(request, [&](int userId) {
            return sendToChat(request, chatId, userId);
        });
    });
    CROW_ROUTE(app, "/chats/<string>/messages").methods(crow::HTTPMethod::GET)([this](const crow::request& request, const std::string& chatId) {
        return authGuard.withUser(request, [&](int userId) {
            return receive(request, chatId, userId);
        });
    });
}

crow::response MessagesController::sendToUser(const crow::request& request, int userId) {
    const std::optional<JsonBody> body = JsonBody::parse(request);
    const std::optional<int> recipientId = body ? body->getInt("recipientId") : std::nullopt;
    const std::optional<std::string> text = body ? body->getString("text") : std::nullopt;
    if (!recipientId.has_value() || !text.has_value()) {
        return HttpResponses::error(400, "recipientId and text are required");
    }
    return toHttpResponse(messagesService.sendMessage(userId, *recipientId, *text));
}

crow::response MessagesController::sendToChat(const crow::request& request, const std::string& chatId, int userId) {
    const std::optional<JsonBody> body = JsonBody::parse(request);
    const std::optional<std::string> text = body ? body->getString("text") : std::nullopt;
    if (!text.has_value()) {
        return HttpResponses::error(400, "text is required");
    }
    return toHttpResponse(messagesService.sendMessage(chatId, userId, *text));
}

crow::response MessagesController::receive(const crow::request& request, const std::string& chatId, int userId) {
    const std::optional<int> afterSeq = readAfterSeq(request);
    if (!afterSeq.has_value()) {
        return HttpResponses::error(400, "afterSeq must be a number");
    }

    const MessageListResponse response = messagesService.receiveMessages(chatId, userId, *afterSeq);
    if (response.not_found()) {
        return HttpResponses::error(404, "Chat not found");
    }
    return HttpResponses::json(200, JsonMapper::toJsonList(*response.response_data));
}
