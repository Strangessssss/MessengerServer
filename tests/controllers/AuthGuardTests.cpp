#include <gtest/gtest.h>

#include <string>
#include <utility>
#include <vector>

#include "tests/support/ControllerFixture.h"

class AuthGuardTests : public ControllerFixture {};

TEST_F(AuthGuardTests, ProtectedRoutesRejectMissingAndInvalidTokens) {
    struct Route {
        HTTPMethod method;
        std::string url;
    };
    const std::vector<Route> routes = {
        {HTTPMethod::GET, "/chats"},
        {HTTPMethod::POST, "/chats"},
        {HTTPMethod::GET, "/chats/chat-1"},
        {HTTPMethod::Delete, "/chats/chat-1"},
        {HTTPMethod::POST, "/chats/chat-1/messages"},
    };

    for (const Route& route : routes) {
        SCOPED_TRACE(route.url);
        expectError(send(route.method, route.url), 401, "Missing or invalid token");
        expectError(send(route.method, route.url, "not-a-token"), 401, "Missing or invalid token");
        expectError(send(route.method, route.url, tokenFor(999)), 401, "Missing or invalid token");
    }
}

TEST_F(AuthGuardTests, ProtectedRoutesRejectAuthorizationWithoutTheBearerPrefix) {
    crow::request request;
    request.method = HTTPMethod::GET;
    request.url = "/chats";
    request.add_header("Authorization", tokenFor(addUser("Alice")));

    crow::response response;
    app.handle_full(request, response);

    expectError(std::move(response), 401, "Missing or invalid token");
}
