#include "HttpResponses.h"

crow::response HttpResponses::json(int status, const crow::json::wvalue& body) {
    crow::response response(status);
    response.set_header("Content-Type", "application/json");
    response.write(body.dump());
    return response;
}

crow::response HttpResponses::error(int status, const std::string& message) {
    crow::json::wvalue body;
    body["error"] = message;
    return json(status, body);
}
