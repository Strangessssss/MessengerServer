#pragma once
#include <chrono>
#include <string>
#include <vector>

struct User {
    int id;

    std::string username;
    std::string passwordHash;

    std::vector<std::string> ChatIds;
    std::vector<int> blockedUserIds;

    std::chrono::system_clock::time_point createdAt;
};
