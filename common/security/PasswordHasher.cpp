#include "PasswordHasher.h"

#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#include <charconv>
#include <optional>
#include <stdexcept>
#include <vector>

#include "../encoding/Encoding.h"

namespace {
    constexpr int iterations = 600000;
    constexpr int saltLength = 16;
    constexpr int hashLength = 32;

    std::string derive(const std::string& password, const std::vector<unsigned char>& salt, int rounds) {
        std::vector<unsigned char> derived(hashLength);
        const int result = PKCS5_PBKDF2_HMAC(password.data(), static_cast<int>(password.size()),
                                             salt.data(), static_cast<int>(salt.size()),
                                             rounds, EVP_sha256(), hashLength, derived.data());
        if (result != 1) {
            throw std::runtime_error("Password hashing failed");
        }
        return Encoding::toHex(derived);
    }
}

std::string PasswordHasher::hash(const std::string& password) {
    std::vector<unsigned char> salt(saltLength);
    if (RAND_bytes(salt.data(), saltLength) != 1) {
        throw std::runtime_error("Salt generation failed");
    }
    return std::to_string(iterations) + "$" + Encoding::toHex(salt) + "$" + derive(password, salt, iterations);
}

bool PasswordHasher::verify(const std::string& password, const std::string& storedHash) {
    const size_t first = storedHash.find('$');
    const size_t second = storedHash.find('$', first + 1);
    if (first == std::string::npos || second == std::string::npos) {
        return false;
    }

    int rounds = 0;
    const auto [end, error] = std::from_chars(storedHash.data(), storedHash.data() + first, rounds);
    const std::optional<std::vector<unsigned char>> salt = Encoding::fromHex(storedHash.substr(first + 1, second - first - 1));
    if (error != std::errc{} || end != storedHash.data() + first || rounds <= 0 || !salt.has_value() || salt->empty()) {
        return false;
    }

    const std::string expected = storedHash.substr(second + 1);
    const std::string actual = derive(password, *salt, rounds);
    return actual.size() == expected.size() && CRYPTO_memcmp(actual.data(), expected.data(), actual.size()) == 0;
}
