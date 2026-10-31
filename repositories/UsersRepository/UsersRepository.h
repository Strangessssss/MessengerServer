#pragma once
#include <optional>

#include "../../database/Database.h"
#include "../../entities/User.h"

class UsersRepository {
    public:
        explicit UsersRepository(Database& database);

        User add(User user);
        std::optional<User> findById(int id) const;
        std::optional<User> findByUsername(const std::string& username) const;

    private:
        Database& database;
};
