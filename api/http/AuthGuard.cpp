#include "AuthGuard.h"

#include <string>

namespace {
    const std::string bearerPrefix = "Bearer ";
}

AuthGuard::AuthGuard(const AuthService& authService) : authService(authService) {}

std::optional<int> AuthGuard::authenticate(const crow::request& request) const {
    const std::string& header = request.get_header_value("Authorization");
    if (!header.starts_with(bearerPrefix)) {
        return std::nullopt;
    }
    return authService.authenticate(header.substr(bearerPrefix.size()));
}
