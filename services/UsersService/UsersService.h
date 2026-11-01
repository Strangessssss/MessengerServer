#pragma once

#include <string>

#include "../../repositories/UsersRepository/UsersRepository.h"
#include "DTOs/responses/UserIdResponse.h"

class UsersService {
    public:
        UsersService(UsersRepository& usersRepository);

        UserIdResponse getUserIdByUsername(const std::string& username) const;
    private:
        UsersRepository& usersRepository;
};
