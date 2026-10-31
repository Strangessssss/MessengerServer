#include "AuthService.h"

#include <chrono>
#include <utility>

#include "../../common/security/PasswordHasher.h"

namespace {
    constexpr size_t minPasswordLength = 8;
}

AuthService::AuthService(UsersRepository& usersRepository, TokenService& tokenService)
    : usersRepository(usersRepository), tokenService(tokenService) {}

AuthResponse AuthService::registerUser(std::string username, const std::string& password) {
    if (username.empty() || password.size() < minPasswordLength || usersRepository.findByUsername(username).has_value()) {
        return AuthResponse(std::nullopt, false);
    }

    User user;
    user.username = std::move(username);
    user.passwordHash = PasswordHasher::hash(password);
    user.createdAt = std::chrono::system_clock::now();

    const User created = usersRepository.add(std::move(user));
    return AuthResponse(AuthResult{created.id, tokenService.createToken(created.id)});
}

AuthResponse AuthService::login(const std::string& username, const std::string& password) {
    const std::optional<User> user = usersRepository.findByUsername(username);
    if (!user.has_value()) {
        // Takes as long as a real check, so timing doesn't reveal which usernames exist.
        PasswordHasher::hash(password);
        return AuthResponse(std::nullopt, false);
    }
    if (!PasswordHasher::verify(password, user->passwordHash)) {
        return AuthResponse(std::nullopt, false);
    }
    return AuthResponse(AuthResult{user->id, tokenService.createToken(user->id)});
}

std::optional<int> AuthService::authenticate(const std::string& token) const {
    const std::optional<int> userId = tokenService.verifyToken(token);
    if (!userId.has_value() || !usersRepository.findById(*userId).has_value()) {
        return std::nullopt;
    }
    return userId;
}
