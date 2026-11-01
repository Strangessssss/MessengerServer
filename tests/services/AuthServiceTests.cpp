#include <gtest/gtest.h>

#include <string>

#include "common/security/PasswordHasher.h"
#include "services/AuthService/AuthService.h"
#include "services/TokenService/TokenService.h"
#include "tests/support/ServiceFixture.h"

namespace {
    const std::string testSecret = "test-secret-that-is-at-least-32-characters";
}

class AuthServiceTests : public ServiceFixture {
    protected:
        TokenService tokenService{testSecret};
        AuthService service{usersRepository, tokenService};
};

TEST_F(AuthServiceTests, RegisterCreatesTheUserAndReturnsAWorkingToken) {
    const AuthResponse response = service.registerUser("Alice", "password123");

    ASSERT_TRUE(response.success);
    ASSERT_TRUE(response.response_data.has_value());

    const auto user = usersRepository.findByUsername("Alice");
    ASSERT_TRUE(user.has_value());
    EXPECT_EQ(response.response_data->userId, user->id);
    EXPECT_EQ(service.authenticate(response.response_data->token), user->id);
}

TEST_F(AuthServiceTests, RegisterStoresAHashAndNotThePassword) {
    service.registerUser("Alice", "password123");

    const auto user = usersRepository.findByUsername("Alice");
    ASSERT_TRUE(user.has_value());
    EXPECT_NE(user->passwordHash, "password123");
    EXPECT_TRUE(PasswordHasher::verify("password123", user->passwordHash));
}

TEST_F(AuthServiceTests, RegisterRejectsEmptyUsername) {
    const AuthResponse response = service.registerUser("", "password123");

    EXPECT_FALSE(response.success);
    EXPECT_FALSE(response.response_data.has_value());
}

TEST_F(AuthServiceTests, RegisterRejectsPasswordUnderEightCharacters) {
    EXPECT_FALSE(service.registerUser("Alice", "1234567").success);
    EXPECT_FALSE(usersRepository.findByUsername("Alice").has_value());
}

TEST_F(AuthServiceTests, RegisterAcceptsPasswordOfEightCharacters) {
    EXPECT_TRUE(service.registerUser("Alice", "12345678").success);
}

TEST_F(AuthServiceTests, RegisterRejectsTakenUsernameAndKeepsTheOriginalUser) {
    const int id = addUser("Alice");

    EXPECT_FALSE(service.registerUser("Alice", "password123").success);

    const auto user = usersRepository.findByUsername("Alice");
    ASSERT_TRUE(user.has_value());
    EXPECT_EQ(user->id, id);
    EXPECT_EQ(user->passwordHash, "passwordHash123");
}

TEST_F(AuthServiceTests, LoginReturnsTheUserAndAWorkingToken) {
    const int id = service.registerUser("Alice", "password123").response_data->userId;

    const AuthResponse response = service.login("Alice", "password123");

    ASSERT_TRUE(response.success);
    ASSERT_TRUE(response.response_data.has_value());
    EXPECT_EQ(response.response_data->userId, id);
    EXPECT_EQ(service.authenticate(response.response_data->token), id);
}

TEST_F(AuthServiceTests, LoginFailsTheSameWayForWrongPasswordAndUnknownUsername) {
    service.registerUser("Alice", "password123");

    const AuthResponse wrongPassword = service.login("Alice", "wrong-password");
    const AuthResponse unknownUsername = service.login("nobody", "password123");

    EXPECT_FALSE(wrongPassword.success);
    EXPECT_FALSE(wrongPassword.response_data.has_value());
    EXPECT_FALSE(unknownUsername.success);
    EXPECT_FALSE(unknownUsername.response_data.has_value());
}

TEST_F(AuthServiceTests, AuthenticateRejectsInvalidToken) {
    EXPECT_FALSE(service.authenticate("not-a-token").has_value());
}

TEST_F(AuthServiceTests, AuthenticateRejectsValidTokenOfUnknownUser) {
    EXPECT_FALSE(service.authenticate(tokenService.createToken(999)).has_value());
}
