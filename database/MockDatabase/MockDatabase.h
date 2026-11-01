#pragma once
#include <unordered_map>

#include "../Database/Database.h"

class MockDatabase : public Database {
    public:
        User insertUser(User user) override;
        std::optional<User> findUserById(int id) const override;
        std::optional<User> findUserByUsername(const std::string& username) const override;

        std::optional<Chat> insertChat(const Chat& chat) override;
        void deleteChat(const std::string& id) override;
        std::optional<Chat> findChatById(const std::string& id) const override;
        std::vector<Chat> findChatsByUserId(int userId) const override;

        void insertMessage(const Message& message) override;
        std::vector<Message> findMessagesByChatId(const std::string& chatId) const override;
        void markMessagesRead(const std::string& chatId, int readerId) override;

    private:
        std::unordered_map<int, User> users;
        std::unordered_map<std::string, Chat> chats;
        std::unordered_map<std::string, Message> messages;
        int nextUserId = 1;
};
