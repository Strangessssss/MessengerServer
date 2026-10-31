#pragma once

#include <optional>
#include <string>
#include <vector>

#include "../../database/Database.h"
#include "../../entities/Message.h"

class MessagesRepository {
    public:
        explicit MessagesRepository(Database& database);

        Message add(Message message);

        std::vector<Message> get_messages(std::string chatId) const;
        std::optional<Message> get_latest_message(std::string chatId) const;

        void mark_received(std::string chatId, int recipientId);
        void mark_read(std::string chatId, int readerId);

    private:
        Database& database;
};
