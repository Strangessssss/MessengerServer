#pragma once
#include <crow.h>

#include <string>

#include "../../services/ChatsService/ChatsService.h"
#include "../../services/UsersService/UsersService.h"
#include "../http/AuthGuard.h"

class ChatsController {
    public:
        ChatsController(ChatsService& chatsService, UsersService& usersService ,const AuthGuard& authGuard);

        void registerRoutes(crow::SimpleApp& app);

    private:
        crow::response getChats(int userId);
        crow::response createChat(const crow::request& request, int userId);
        crow::response getChatView(const std::string& chatId, int userId);
        crow::response deleteChat(const std::string& chatId, int userId);

        ChatsService& chatsService;
        UsersService& usersService;
        const AuthGuard& authGuard;
};
