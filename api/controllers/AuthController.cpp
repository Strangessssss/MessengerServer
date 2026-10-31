#include "AuthController.h"

#include <optional>
#include <string>

#include "../http/HttpResponses.h"
#include "../http/JsonBody.h"
#include "../json/JsonMapper.h"

AuthController::AuthController(AuthService& authService) : authService(authService) {}

void AuthController::registerRoutes(crow::SimpleApp& app) {
    CROW_ROUTE(app, "/auth/register").methods(crow::HTTPMethod::POST)([this](const crow::request& request) {
        return registerUser(request);
    });
    CROW_ROUTE(app, "/auth/login").methods(crow::HTTPMethod::POST)([this](const crow::request& request) {
        return login(request);
    });
}

crow::response AuthController::registerUser(const crow::request& request) {
    const std::optional<JsonBody> body = JsonBody::parse(request);
    const std::optional<std::string> username = body ? body->getString("username") : std::nullopt;
    const std::optional<std::string> password = body ? body->getString("password") : std::nullopt;
    if (!username.has_value() || !password.has_value()) {
        return HttpResponses::error(400, "username and password are required");
    }

    const AuthResponse response = authService.registerUser(*username, *password);
    if (!response.success_) {
        return HttpResponses::error(400, "Username is taken or password is too short");
    }
    return HttpResponses::json(201, JsonMapper::toJson(*response.response_data));
}

crow::response AuthController::login(const crow::request& request) {
    const std::optional<JsonBody> body = JsonBody::parse(request);
    const std::optional<std::string> username = body ? body->getString("username") : std::nullopt;
    const std::optional<std::string> password = body ? body->getString("password") : std::nullopt;
    if (!username.has_value() || !password.has_value()) {
        return HttpResponses::error(400, "username and password are required");
    }

    const AuthResponse response = authService.login(*username, *password);
    if (!response.success_) {
        return HttpResponses::error(401, "Invalid username or password");
    }
    return HttpResponses::json(200, JsonMapper::toJson(*response.response_data));
}
