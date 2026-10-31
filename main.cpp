#include <crow.h>

#include <cstdlib>
#include <iostream>
#include <string>

#include "api/controllers/AuthController.h"
#include "api/controllers/ChatsController.h"
#include "api/controllers/MessagesController.h"
#include "api/http/AuthGuard.h"
#include "database/MockDatabase.h"
#include "repositories/ChatsRepository/ChatsRepository.h"
#include "repositories/MessagesRepository/MessagesRepository.h"
#include "repositories/UsersRepository/UsersRepository.h"
#include "services/AuthService/AuthService.h"
#include "services/ChatsService/ChatsService.h"
#include "services/MessagesService/MessagesService.h"
#include "services/TokenService/TokenService.h"

namespace {
    constexpr int port = 8080;

    // One thread accepts connections and one runs handlers, so handlers never overlap.
    // MockDatabase and the services' check-then-write steps rely on this.
    constexpr int threadCount = 2;
}

int main() {
    const char* jwtSecret = std::getenv("JWT_SECRET");
    if (jwtSecret == nullptr || std::string(jwtSecret).size() < 32) {
        std::cerr << "JWT_SECRET must be set to at least 32 characters" << std::endl;
        return 1;
    }

    MockDatabase database;
    UsersRepository usersRepository(database);
    ChatsRepository chatsRepository(database);
    MessagesRepository messagesRepository(database);

    TokenService tokenService(jwtSecret);
    AuthService authService(usersRepository, tokenService);
    ChatsService chatsService(chatsRepository, messagesRepository, usersRepository);
    MessagesService messagesService(messagesRepository, chatsRepository, usersRepository);

    AuthGuard authGuard(authService);
    AuthController authController(authService);
    ChatsController chatsController(chatsService, authGuard);
    MessagesController messagesController(messagesService, authGuard);

    crow::SimpleApp app;
    authController.registerRoutes(app);
    chatsController.registerRoutes(app);
    messagesController.registerRoutes(app);

    app.port(port).concurrency(threadCount).run();
    return 0;
}
