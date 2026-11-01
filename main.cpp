#include <crow.h>
#include <Guid.hpp>
#include <charconv>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <optional>
#include <string>
#include <system_error>

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

namespace {
    constexpr int defaultPort = 8080;

    // One thread accepts connections and one runs handlers, so handlers never overlap.
    // MockDatabase and the services' check-then-write steps rely on this.
    constexpr int threadCount = 2;

    // Responds with the default if PORT isn't set, and with nothing if it isn't a valid port.
    std::optional<int> readPort() {
        const char* value = std::getenv("PORT");
        if (value == nullptr) {
            return defaultPort;
        }

        int port = 0;
        const char* end = value + std::strlen(value);
        const auto [parsedEnd, error] = std::from_chars(value, end, port);
        if (error != std::errc{} || parsedEnd != end || port < 1 || port > 65535) {
            return std::nullopt;
        }
        return port;
    }
}

int main() {
    const char* jwtSecret = std::getenv("JWT_SECRET");
    if (jwtSecret == nullptr || std::string(jwtSecret).size() < 32) {
        std::cerr << "JWT_SECRET must be set to at least 32 characters" << std::endl;
        return 1;
    }

    const std::optional<int> port = readPort();
    if (!port.has_value()) {
        std::cerr << "PORT must be a number from 1 to 65535" << std::endl;
        return 1;
    }

    MockDatabase database;
    UsersRepository usersRepository(database);
    ChatsRepository chatsRepository(database);
    MessagesRepository messagesRepository(database);

    // The database starts empty in each process, so user IDs repeat. A signing key unique to this
    // process makes older tokens invalid, instead of letting them match a new user with the same ID.
    TokenService tokenService(std::string(jwtSecret) + xg::newGuid().str());
    AuthService authService(usersRepository, tokenService);
    ChatsService chatsService(chatsRepository, messagesRepository, usersRepository);
    MessagesService messagesService(messagesRepository, chatsRepository);
    UsersService usersService(usersRepository);

    AuthGuard authGuard(authService);
    AuthController authController(authService);
    ChatsController chatsController(chatsService, usersService, authGuard);
    MessagesController messagesController(messagesService, authGuard);

    crow::SimpleApp app;
    authController.registerRoutes(app);
    chatsController.registerRoutes(app);
    messagesController.registerRoutes(app);

    app.port(static_cast<std::uint16_t>(*port)).concurrency(threadCount).run();
    return 0;
}
