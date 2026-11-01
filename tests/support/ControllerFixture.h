#pragma once
#include <crow.h>
#include <gtest/gtest.h>

#include <optional>
#include <string>

#include "api/controllers/AuthController/AuthController.h"
#include "api/controllers/ChatsController/ChatsController.h"
#include "api/controllers/MessagesController/MessagesController.h"
#include "api/http/AuthGuard/AuthGuard.h"
#include "database/MockDatabase/MockDatabase.h"
#include "repositories/ChatsRepository/ChatsRepository.h"
#include "repositories/MessagesRepository/MessagesRepository.h"
#include "repositories/UsersRepository/UsersRepository.h"
#include "services/AuthService/AuthService.h"
#include "services/ChatsService/ChatsService.h"
#include "services/MessagesService/MessagesService.h"
#include "services/TokenService/TokenService.h"
#include "services/UsersService/UsersService.h"

inline std::string str(const crow::json::rvalue& value) {
    return std::string(value.s());
}

// The full server wired as in main.cpp, with requests sent straight to Crow's router.
class ControllerFixture : public testing::Test {
    protected:
        using HTTPMethod = crow::HTTPMethod;

        ControllerFixture() {
            app.loglevel(crow::LogLevel::Warning);
            authController.registerRoutes(app);
            chatsController.registerRoutes(app);
            messagesController.registerRoutes(app);
            app.validate();
        }

        crow::response send(HTTPMethod method, const std::string& url,
                            const std::optional<std::string>& token = std::nullopt,
                            const std::string& body = "") {
            crow::request request;
            request.method = method;
            request.url = url;
            request.body = body;
            if (token.has_value()) {
                request.add_header("Authorization", "Bearer " + *token);
            }

            crow::response response;
            app.handle_full(request, response);
            return response;
        }

        // Skips password hashing, which is slow and only matters to the auth routes.
        int addUser(const std::string& username) {
            User user;
            user.username = username;
            user.passwordHash = "unused";
            return usersRepository.add(user).id;
        }

        std::string tokenFor(int userId) const {
            return tokenService.createToken(userId);
        }

        static void expectError(crow::response&& response, int status, const std::string& message) {
            EXPECT_EQ(response.code, status);
            EXPECT_EQ(response.get_header_value("Content-Type"), "application/json");
            const crow::json::rvalue json = crow::json::load(response.body);
            ASSERT_TRUE(static_cast<bool>(json));
            EXPECT_EQ(str(json["error"]), message);
        }

        MockDatabase database;
        UsersRepository usersRepository{database};
        ChatsRepository chatsRepository{database};
        MessagesRepository messagesRepository{database};

        TokenService tokenService{"test-secret-that-is-at-least-32-characters"};
        AuthService authService{usersRepository, tokenService};
        ChatsService chatsService{chatsRepository, messagesRepository, usersRepository};
        MessagesService messagesService{messagesRepository, chatsRepository};
        UsersService usersService{usersRepository};

        AuthGuard authGuard{authService};
        AuthController authController{authService};
        ChatsController chatsController{chatsService, usersService, authGuard};
        MessagesController messagesController{messagesService, authGuard};

        crow::SimpleApp app;
};

// Two users who can share a chat, plus a third who isn't in it.
class ChatRouteFixture : public ControllerFixture {
    protected:
        ChatRouteFixture() {
            alice = addUser("Alice");
            bob = addUser("Bob");
            carol = addUser("Carol");
            aliceToken = tokenFor(alice);
            bobToken = tokenFor(bob);
            carolToken = tokenFor(carol);
        }

        std::string createChat() {
            const crow::response response = send(HTTPMethod::POST, "/chats", aliceToken, R"({"username":"Bob"})");
            return str(crow::json::load(response.body)["id"]);
        }

        int alice = 0;
        int bob = 0;
        int carol = 0;
        std::string aliceToken;
        std::string bobToken;
        std::string carolToken;
};
