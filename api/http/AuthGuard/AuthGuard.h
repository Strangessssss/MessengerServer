#pragma once
#include <crow.h>

#include <optional>

#include "../../../services/AuthService/AuthService.h"
#include "../HttpResponses/HttpResponses.h"

// Runs a handler as the user identified by the request's bearer token, or responds with 401.
class AuthGuard {
    public:
        explicit AuthGuard(const AuthService& authService);

        template <typename Handler>
        crow::response withUser(const crow::request& request, Handler handler) const {
            const std::optional<int> userId = authenticate(request);
            if (!userId.has_value()) {
                return HttpResponses::error(401, "Missing or invalid token");
            }
            return handler(*userId);
        }

    private:
        std::optional<int> authenticate(const crow::request& request) const;

        const AuthService& authService;
};
