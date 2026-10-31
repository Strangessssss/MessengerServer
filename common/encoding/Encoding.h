#pragma once
#include <optional>
#include <string>
#include <vector>

class Encoding {
    public:
        // Unpadded URL-safe Base64, as JWTs use.
        static std::string base64UrlEncode(const std::string& input);
        static std::optional<std::string> base64UrlDecode(const std::string& input);

        static std::string toHex(const std::vector<unsigned char>& bytes);
        static std::optional<std::vector<unsigned char>> fromHex(const std::string& hex);
};
