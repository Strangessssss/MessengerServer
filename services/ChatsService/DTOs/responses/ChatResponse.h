#pragma once

#include "../../../../entities/Chat.h"
#include "../../../../common/response/Response.h"

struct ChatResponse: Response<Chat> {
    using Response::Response;
};
