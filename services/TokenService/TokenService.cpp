#include "TokenService.h"

#include <Guid.hpp>
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>

#include <charconv>
#include <stdexcept>
#include <utility>

#include "../../common/encoding/Encoding.h"

namespace {
    // Only reads payloads this service signed, so the claim layout is known.
    template <typename T>
    std::optional<T> readNumberClaim(const std::string& payload, const std::string& key) {
        const size_t position = payload.find(key);
        if (position == std::string::npos) {
            return std::nullopt;
        }
        const char* begin = payload.data() + position + key.size();
        T value{};
        const auto [end, error] = std::from_chars(begin, payload.data() + payload.size(), value);
        if (error != std::errc{}) {
            return std::nullopt;
        }
        return value;
    }

    const std::string encodedHeader = Encoding::base64UrlEncode(R"({"alg":"HS256","typ":"JWT"})");
}

TokenService::TokenService(std::string secret, std::chrono::seconds lifetime)
    : secret(std::move(secret)), lifetime(lifetime) {}

std::string TokenService::createToken(int userId) const {
    const auto now = std::chrono::system_clock::now();
    const auto issuedAt = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());
    const auto expiresAt = issuedAt + lifetime;

    const std::string payload = R"({"sub":")" + std::to_string(userId)
        + R"(","jti":")" + xg::newGuid().str()
        + R"(","iat":)" + std::to_string(issuedAt.count())
        + R"(,"exp":)" + std::to_string(expiresAt.count()) + "}";

    const std::string data = encodedHeader + "." + Encoding::base64UrlEncode(payload);
    return data + "." + sign(data);
}

std::optional<int> TokenService::verifyToken(const std::string& token) const {
    const size_t first = token.find('.');
    if (first == std::string::npos) {
        return std::nullopt;
    }
    const size_t second = token.find('.', first + 1);
    if (second == std::string::npos || token.find('.', second + 1) != std::string::npos) {
        return std::nullopt;
    }

    // The header must match exactly, so a token can't pick its own algorithm.
    if (token.substr(0, first) != encodedHeader) {
        return std::nullopt;
    }

    const std::string expected = sign(token.substr(0, second));
    const std::string actual = token.substr(second + 1);
    if (actual.size() != expected.size() || CRYPTO_memcmp(actual.data(), expected.data(), actual.size()) != 0) {
        return std::nullopt;
    }

    const std::optional<std::string> payload = Encoding::base64UrlDecode(token.substr(first + 1, second - first - 1));
    if (!payload.has_value()) {
        return std::nullopt;
    }

    const std::optional<int> userId = readNumberClaim<int>(*payload, R"("sub":")");
    const std::optional<long long> expiresAt = readNumberClaim<long long>(*payload, R"("exp":)");
    if (!userId.has_value() || !expiresAt.has_value()) {
        return std::nullopt;
    }

    const auto now = std::chrono::system_clock::now();
    const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());
    if (seconds.count() >= *expiresAt) {
        return std::nullopt;
    }
    return userId;
}

std::string TokenService::sign(const std::string& data) const {
    std::string signature(EVP_MAX_MD_SIZE, '\0');
    unsigned int length = 0;
    const unsigned char* result = HMAC(EVP_sha256(), secret.data(), static_cast<int>(secret.size()),
                                       reinterpret_cast<const unsigned char*>(data.data()), data.size(),
                                       reinterpret_cast<unsigned char*>(signature.data()), &length);
    if (result == nullptr) {
        throw std::runtime_error("Token signing failed");
    }
    signature.resize(length);
    return Encoding::base64UrlEncode(signature);
}
