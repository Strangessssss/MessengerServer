#pragma once
#include <crow.h>

#include <string>

namespace HttpResponses {
    crow::response json(int status, const crow::json::wvalue& body);
    crow::response error(int status, const std::string& message);
}
