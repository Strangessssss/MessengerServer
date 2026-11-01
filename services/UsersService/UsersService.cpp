//
// Created by Tima on 10/31/2026.
//

#include "UsersService.h"

UsersService::UsersService(UsersRepository& usersRepository) : usersRepository(usersRepository) {}

UserIdResponse UsersService::getUserIdByUsername(const std::string& username) const {
    const auto user = usersRepository.findByUsername(username);
    if (!user.has_value()) {
        return UserIdResponse(std::nullopt);
    }
    return UserIdResponse(user->id);
}
