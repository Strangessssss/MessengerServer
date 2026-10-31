#pragma once

#include "../../../../common/response/Response.h"
#include "../../../../entities/Message.h"

struct MessageResponse: Response<Message> {
    using Response::Response;
};
