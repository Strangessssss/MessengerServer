#pragma once
#include <optional>
#include <string>

#include "../../repositories/UsersRepository/UsersRepository.h"
#include "../TokenService/TokenService.h"
#include "DTOs/responses/AuthResponse.h"

class AuthService {
    public:
        AuthService(UsersRepository& usersRepository, TokenService& tokenService);

        // Responds with a failure if the username is empty or taken, or the password is too short.
        AuthResponse registerUser(std::string username, const std::string& password);

        // Responds with the same failure for an unknown username and a wrong password.
        AuthResponse login(const std::string& username, const std::string& password);

        // Responds with the user ID, or nothing if the token is invalid or its user doesn't exist.
        std::optional<int> authenticate(const std::string& token) const;

    private:
        UsersRepository& usersRepository;
        TokenService& tokenService;
};
