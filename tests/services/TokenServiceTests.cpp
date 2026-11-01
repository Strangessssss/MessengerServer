#include <gtest/gtest.h>

#include <chrono>
#include <string>
#include <vector>

#include "common/encoding/Encoding.h"
#include "services/TokenService/TokenService.h"

namespace {
    const std::string testSecret = "test-secret-that-is-at-least-32-characters";

    // Splits "header.payload.signature". The token must have exactly three parts.
    std::vector<std::string> splitToken(const std::string& token) {
        const size_t first = token.find('.');
        const size_t second = token.find('.', first + 1);
        return {token.substr(0, first), token.substr(first + 1, second - first - 1), token.substr(second + 1)};
    }
}

TEST(TokenServiceTests, CreatedTokenVerifiesToTheSameUser) {
    const TokenService service(testSecret);

    const auto userId = service.verifyToken(service.createToken(42));

    ASSERT_TRUE(userId.has_value());
    EXPECT_EQ(*userId, 42);
}

TEST(TokenServiceTests, TokensForTheSameUserAreUnique) {
    const TokenService service(testSecret);

    EXPECT_NE(service.createToken(42), service.createToken(42));
}

TEST(TokenServiceTests, RejectsTokenSignedWithAnotherSecret) {
    const TokenService service(testSecret);
    const TokenService otherService("another-secret-that-is-at-least-32-characters");

    EXPECT_FALSE(service.verifyToken(otherService.createToken(42)).has_value());
}

TEST(TokenServiceTests, RejectsExpiredToken) {
    const TokenService service(testSecret, std::chrono::seconds(-1));

    EXPECT_FALSE(service.verifyToken(service.createToken(42)).has_value());
}

TEST(TokenServiceTests, RejectsTokenWithAChangedPayload) {
    const TokenService service(testSecret);
    const auto parts = splitToken(service.createToken(42));
    const std::string forgedPayload = Encoding::base64UrlEncode(R"({"sub":"1","exp":99999999999})");

    EXPECT_FALSE(service.verifyToken(parts[0] + "." + forgedPayload + "." + parts[2]).has_value());
}

TEST(TokenServiceTests, RejectsTokenWithAChangedSignature) {
    const TokenService service(testSecret);
    const auto parts = splitToken(service.createToken(42));
    const std::string forgedSignature(parts[2].size(), 'A');

    EXPECT_FALSE(service.verifyToken(parts[0] + "." + parts[1] + "." + forgedSignature).has_value());
}

TEST(TokenServiceTests, RejectsTokenThatPicksItsOwnAlgorithm) {
    const TokenService service(testSecret);
    const auto parts = splitToken(service.createToken(42));
    const std::string noneHeader = Encoding::base64UrlEncode(R"({"alg":"none","typ":"JWT"})");

    EXPECT_FALSE(service.verifyToken(noneHeader + "." + parts[1] + "." + parts[2]).has_value());
    EXPECT_FALSE(service.verifyToken(noneHeader + "." + parts[1] + ".").has_value());
}

TEST(TokenServiceTests, RejectsMalformedTokens) {
    const TokenService service(testSecret);
    const std::string validToken = service.createToken(42);

    for (const std::string& token : {std::string(""), std::string("abc"), std::string("a.b"), std::string("a.b.c"), validToken + ".extra"}) {
        SCOPED_TRACE(token);
        EXPECT_FALSE(service.verifyToken(token).has_value());
    }
}
