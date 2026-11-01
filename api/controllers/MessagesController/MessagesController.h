#pragma once
#include <crow.h>

#include <string>

#include "../../../services/MessagesService/MessagesService.h"
#include "../../http/AuthGuard/AuthGuard.h"

class MessagesController {
    public:
        MessagesController(MessagesService& messagesService, const AuthGuard& authGuard);

        void registerRoutes(crow::SimpleApp& app);

    private:
        crow::response sendToChat(const crow::request& request, const std::string& chatId, int userId);

        MessagesService& messagesService;
        const AuthGuard& authGuard;
};
