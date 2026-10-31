#pragma once

#include "../../../../common/response/Response.h"

struct UserIdResponse: Response<int> {
    using Response::Response;
};
