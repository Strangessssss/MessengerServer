#pragma once
#include <string>

#include "../../../../common/response/Response.h"

struct AuthResult {
    int userId;
    std::string token;
};

struct AuthResponse: Response<AuthResult> {
    using Response::Response;
};
