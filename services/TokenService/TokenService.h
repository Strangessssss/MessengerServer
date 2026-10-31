#pragma once
#include <chrono>
#include <optional>
#include <string>

// Issues and verifies HS256 JWTs. The GUID is the token's "jti" claim.
class TokenService {
    public:
        explicit TokenService(std::string secret, std::chrono::seconds lifetime = std::chrono::hours(24));

        std::string createToken(int userId) const;

        // Responds with the user ID, or nothing if the token is invalid or expired.
        std::optional<int> verifyToken(const std::string& token) const;

    private:
        std::string sign(const std::string& data) const;

        std::string secret;
        std::chrono::seconds lifetime;
};
