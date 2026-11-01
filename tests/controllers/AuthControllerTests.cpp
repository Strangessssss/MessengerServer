#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "tests/support/ControllerFixture.h"

class AuthControllerTests : public ControllerFixture {};

// /auth/register

TEST_F(AuthControllerTests, RegisterRespondsCreatedWithAUserIdAndAWorkingToken) {
    crow::response response = send(HTTPMethod::POST, "/auth/register", std::nullopt,
                                   R"({"username":"Alice","password":"password123"})");

    EXPECT_EQ(response.code, 201);
    EXPECT_EQ(response.get_header_value("Content-Type"), "application/json");

    const crow::json::rvalue json = crow::json::load(response.body);
    ASSERT_TRUE(static_cast<bool>(json));
    EXPECT_EQ(json["userId"].i(), usersRepository.findByUsername("Alice")->id);
    EXPECT_EQ(send(HTTPMethod::GET, "/chats", str(json["token"])).code, 200);
}

TEST_F(AuthControllerTests, RegisterRejectsBadBodies) {
    const std::vector<std::string> bodies = {
        "",
        "not json",
        "[]",
        R"({"username":"Alice"})",
        R"({"password":"password123"})",
        R"({"username":1,"password":"password123"})",
        R"({"username":"Alice","password":1})",
    };

    for (const std::string& body : bodies) {
        SCOPED_TRACE(body);
        expectError(send(HTTPMethod::POST, "/auth/register", std::nullopt, body), 400, "username and password are required");
    }
    EXPECT_FALSE(usersRepository.findByUsername("Alice").has_value());
}

TEST_F(AuthControllerTests, RegisterRejectsTakenUsernameAndShortPassword) {
    addUser("Alice");

    expectError(send(HTTPMethod::POST, "/auth/register", std::nullopt, R"({"username":"Alice","password":"password123"})"),
                400, "Username is taken or password is too short");
    expectError(send(HTTPMethod::POST, "/auth/register", std::nullopt, R"({"username":"Bob","password":"short"})"),
                400, "Username is taken or password is too short");
}

// /auth/login

TEST_F(AuthControllerTests, LoginRespondsOkWithAUserIdAndAWorkingToken) {
    const std::string credentials = R"({"username":"Alice","password":"password123"})";
    send(HTTPMethod::POST, "/auth/register", std::nullopt, credentials);

    const crow::response response = send(HTTPMethod::POST, "/auth/login", std::nullopt, credentials);

    EXPECT_EQ(response.code, 200);

    const crow::json::rvalue json = crow::json::load(response.body);
    ASSERT_TRUE(static_cast<bool>(json));
    EXPECT_EQ(json["userId"].i(), usersRepository.findByUsername("Alice")->id);
    EXPECT_EQ(send(HTTPMethod::GET, "/chats", str(json["token"])).code, 200);
}

TEST_F(AuthControllerTests, LoginGivesTheSameErrorForWrongPasswordAndUnknownUsername) {
    send(HTTPMethod::POST, "/auth/register", std::nullopt, R"({"username":"Alice","password":"password123"})");

    expectError(send(HTTPMethod::POST, "/auth/login", std::nullopt, R"({"username":"Alice","password":"wrong-password"})"),
                401, "Invalid username or password");
    expectError(send(HTTPMethod::POST, "/auth/login", std::nullopt, R"({"username":"nobody","password":"password123"})"),
                401, "Invalid username or password");
}

TEST_F(AuthControllerTests, LoginRejectsBadBodies) {
    expectError(send(HTTPMethod::POST, "/auth/login", std::nullopt, "not json"), 400, "username and password are required");
    expectError(send(HTTPMethod::POST, "/auth/login", std::nullopt, R"({"username":"Alice"})"), 400, "username and password are required");
}
