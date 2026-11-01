#pragma once
#include <algorithm>
#include <chrono>
#include <string>
#include <vector>

struct Chat {
    std::string id;

    std::vector<int> userIds;

    std::chrono::system_clock::time_point createdAt;

    bool hasUser(int userId) const {
        return std::ranges::find(userIds, userId) != userIds.end();
    }
};