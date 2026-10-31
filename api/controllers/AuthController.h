#pragma once
#include <crow.h>

#include "../../services/AuthService/AuthService.h"

class AuthController {
    public:
        explicit AuthController(AuthService& authService);

        void registerRoutes(crow::SimpleApp& app);

    private:
        crow::response registerUser(const crow::request& request);
        crow::response login(const crow::request& request);

        AuthService& authService;
};
