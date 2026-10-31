#include "UsersRepository.h"

#include <utility>

UsersRepository::UsersRepository(Database& database) : database(database) {}

User UsersRepository::add(User user) {
    return database.insertUser(std::move(user));
}

std::optional<User> UsersRepository::findById(int id) const {
    return database.findUserById(id);
}

std::optional<User> UsersRepository::findByUsername(const std::string& username) const {
    return database.findUserByUsername(username);
}
