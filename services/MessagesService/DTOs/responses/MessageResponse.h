#pragma once

#include "../../../../common/response/Response.h"
#include "../../../../entities/Message/Message.h"

struct MessageResponse: Response<Message> {
    using Response::Response;
};
