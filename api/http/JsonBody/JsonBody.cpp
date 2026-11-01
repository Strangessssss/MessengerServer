#include "JsonBody.h"

#include <utility>

JsonBody::JsonBody(crow::json::rvalue value) : value(std::move(value)) {}

std::optional<JsonBody> JsonBody::parse(const crow::request& request) {
    crow::json::rvalue value = crow::json::load(request.body);
    if (!value || value.t() != crow::json::type::Object) {
        return std::nullopt;
    }
    return JsonBody(std::move(value));
}

std::optional<std::string> JsonBody::getString(const std::string& key) const {
    if (!value.has(key) || value[key].t() != crow::json::type::String) {
        return std::nullopt;
    }
    return std::string(value[key].s());
}
