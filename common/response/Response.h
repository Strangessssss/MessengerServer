#pragma once
#include <optional>
#include <utility>

template <typename T>
struct Response {
    explicit Response(std::optional<T> data, bool success = true) : success(success), response_data(std::move(data)) {}

    bool not_found() const { return success && !response_data.has_value(); }

    const bool success;
    std::optional<T> response_data;
};