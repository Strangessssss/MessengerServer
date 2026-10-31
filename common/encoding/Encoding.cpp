#include "Encoding.h"

#include <openssl/evp.h>

#include <algorithm>
#include <charconv>

std::string Encoding::base64UrlEncode(const std::string& input) {
    // EVP_EncodeBlock also writes a terminating null.
    std::string output(4 * ((input.size() + 2) / 3) + 1, '\0');
    const int length = EVP_EncodeBlock(reinterpret_cast<unsigned char*>(output.data()),
                                       reinterpret_cast<const unsigned char*>(input.data()),
                                       static_cast<int>(input.size()));
    output.resize(length);

    std::ranges::replace(output, '+', '-');
    std::ranges::replace(output, '/', '_');
    std::erase(output, '=');
    return output;
}

std::optional<std::string> Encoding::base64UrlDecode(const std::string& input) {
    if (std::ranges::any_of(input, [](char c) { return c == '+' || c == '/' || c == '='; })) {
        return std::nullopt;
    }

    const size_t padding = (4 - input.size() % 4) % 4;
    if (padding == 3) {
        return std::nullopt;
    }

    std::string encoded = input + std::string(padding, '=');
    std::ranges::replace(encoded, '-', '+');
    std::ranges::replace(encoded, '_', '/');

    std::string output(encoded.size() / 4 * 3, '\0');
    const int length = EVP_DecodeBlock(reinterpret_cast<unsigned char*>(output.data()),
                                       reinterpret_cast<const unsigned char*>(encoded.data()),
                                       static_cast<int>(encoded.size()));
    if (length < 0) {
        return std::nullopt;
    }

    // EVP_DecodeBlock counts padding as zero bytes.
    output.resize(length - padding);
    return output;
}

std::string Encoding::toHex(const std::vector<unsigned char>& bytes) {
    std::string hex;
    for (const unsigned char byte : bytes) {
        hex += "0123456789abcdef"[byte >> 4];
        hex += "0123456789abcdef"[byte & 0x0f];
    }
    return hex;
}

std::optional<std::vector<unsigned char>> Encoding::fromHex(const std::string& hex) {
    if (hex.size() % 2 != 0) {
        return std::nullopt;
    }

    std::vector<unsigned char> bytes;
    for (size_t i = 0; i < hex.size(); i += 2) {
        unsigned char byte = 0;
        const auto [end, error] = std::from_chars(hex.data() + i, hex.data() + i + 2, byte, 16);
        if (error != std::errc{} || end != hex.data() + i + 2) {
            return std::nullopt;
        }
        bytes.push_back(byte);
    }
    return bytes;
}
