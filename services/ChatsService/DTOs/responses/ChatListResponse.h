#pragma once
#include <vector>

#include "../../../../entities/Chat/Chat.h"
#include "../../../../common/response/Response.h"

struct ChatListResponse: Response<std::vector<Chat>> {
    using Response::Response;
};
