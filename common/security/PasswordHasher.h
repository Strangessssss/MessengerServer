#pragma once
#include <string>

class PasswordHasher {
    public:
        // Salted PBKDF2-HMAC-SHA256, stored as "iterations$saltHex$hashHex".
        static std::string hash(const std::string& password);
        static bool verify(const std::string& password, const std::string& storedHash);
};
