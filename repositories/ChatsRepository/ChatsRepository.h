#pragma once

#include <optional>
#include <string>
#include <vector>

#include "../../database/Database/Database.h"
#include "../../entities/Chat/Chat.h"

class ChatsRepository {
    public:
        explicit ChatsRepository(Database& database);

        Chat add(Chat chat);

        std::optional<std::vector<Chat>> get_chats(int userId) const;

        void delete_chat(std::string chatId);
        void delete_chat(int userId);

        std::optional<Chat> get_chat(std::string chatId) const;
        std::optional<Chat> find_direct_chat(int firstUserId, int secondUserId) const;

    private:
        Database& database;
};
