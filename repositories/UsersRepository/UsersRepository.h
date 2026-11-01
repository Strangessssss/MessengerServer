#pragma once
#include <optional>

#include "../../database/Database/Database.h"
#include "../../entities/User/User.h"

class UsersRepository {
    public:
        explicit UsersRepository(Database& database);

        User add(User user);
        std::optional<User> findById(int id) const;
        std::optional<User> findByUsername(const std::string& username) const;

    private:
        Database& database;
};
