#pragma once
#include <optional>
#include <string>
#include <vector>

#include "../entities/Chat.h"
#include "../entities/Message.h"
#include "../entities/User.h"

class Database {
    public:
        virtual ~Database() = default;

        virtual User insertUser(User user) = 0;
        virtual std::optional<User> findUserById(int id) const = 0;
        virtual std::optional<User> findUserByUsername(const std::string& username) const = 0;

        virtual void insertChat(const Chat& chat) = 0;
        virtual void deleteChat(const std::string& id) = 0;
        virtual std::optional<Chat> findChatById(const std::string& id) const = 0;
        virtual std::vector<Chat> findChatsByUserId(int userId) const = 0;

        virtual void insertMessage(const Message& message) = 0;
        virtual std::vector<Message> findMessagesByChatId(const std::string& chatId) const = 0;
        virtual void markMessagesReceived(const std::string& chatId, int recipientId) = 0;
        virtual void markMessagesRead(const std::string& chatId, int readerId) = 0;
};
