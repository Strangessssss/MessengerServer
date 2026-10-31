#include "JsonBody.h"

#include <limits>
#include <utility>

namespace {
    std::optional<int> toInt(const crow::json::rvalue& value) {
        if (value.t() != crow::json::type::Number || value.nt() == crow::json::num_type::Floating_point) {
            return std::nullopt;
        }
        const int64_t number = value.i();
        if (number < std::numeric_limits<int>::min() || number > std::numeric_limits<int>::max()) {
            return std::nullopt;
        }
        return static_cast<int>(number);
    }
}

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

std::optional<int> JsonBody::getInt(const std::string& key) const {
    if (!value.has(key)) {
        return std::nullopt;
    }
    return toInt(value[key]);
}

std::optional<std::vector<int>> JsonBody::getIntList(const std::string& key) const {
    if (!value.has(key) || value[key].t() != crow::json::type::List) {
        return std::nullopt;
    }

    std::vector<int> numbers;
    for (const crow::json::rvalue& item : value[key]) {
        const std::optional<int> number = toInt(item);
        if (!number.has_value()) {
            return std::nullopt;
        }
        numbers.push_back(*number);
    }
    return numbers;
}
