#pragma once
#include <vector>

#include "../../../../common/response/Response.h"
#include "../../../../entities/Message.h"

struct MessageListResponse: Response<std::vector<Message>> {
    using Response::Response;
};
