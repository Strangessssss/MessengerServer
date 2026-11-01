#pragma once
#include <crow.h>

#include <optional>
#include <string>

// Typed access to a JSON object request body. A missing or mistyped field reads as nothing.
class JsonBody {
    public:
        static std::optional<JsonBody> parse(const crow::request& request);

        std::optional<std::string> getString(const std::string& key) const;

    private:
        explicit JsonBody(crow::json::rvalue value);

        crow::json::rvalue value;
};
