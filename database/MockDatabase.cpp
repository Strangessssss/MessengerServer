#include "MockDatabase.h"

#include <algorithm>

User MockDatabase::insertUser(User user) {
    user.id = nextUserId++;

    users.emplace(user.id, user);
    return user;
}

std::optional<User> MockDatabase::findUserById(int id) const {
    const auto it = users.find(id);
    if (it == users.end()) {
        return std::nullopt;
    }
    return it->second;
}

std::optional<User> MockDatabase::findUserByUsername(const std::string& username) const {
    for (const auto& [id, user] : users) {
        if (user.username == username) {
            return user;
        }
    }
    return std::nullopt;
}

void MockDatabase::insertChat(const Chat& chat) {
    chats.emplace(chat.id, chat);
}

void MockDatabase::deleteChat(const std::string& id) {
    chats.erase(id);
    std::erase_if(messages, [&id](const auto& entry) {
        return entry.second.chatId == id;
    });
}

std::optional<Chat> MockDatabase::findChatById(const std::string& id) const {
    const auto it = chats.find(id);
    if (it == chats.end()) {
        return std::nullopt;
    }
    return it->second;
}

std::vector<Chat> MockDatabase::findChatsByUserId(int userId) const {
    std::vector<Chat> userChats;
    for (const auto& [id, chat] : chats) {
        if (std::ranges::find(chat.userIds, userId) != chat.userIds.end()) {
            userChats.push_back(chat);
        }
    }
    return userChats;
}

void MockDatabase::insertMessage(const Message& message) {
    messages.emplace(message.id, message);
}

std::vector<Message> MockDatabase::findMessagesByChatId(const std::string& chatId) const {
    std::vector<Message> chatMessages;
    for (const auto& [id, message] : messages) {
        if (message.chatId == chatId) {
            chatMessages.push_back(message);
        }
    }
    std::ranges::sort(chatMessages, {}, &Message::seq);
    return chatMessages;
}

void MockDatabase::markMessagesReceived(const std::string& chatId, int recipientId) {
    for (auto& [id, message] : messages) {
        if (message.chatId == chatId && message.sender != recipientId && message.status == Sent) {
            message.status = Received;
        }
    }
}

void MockDatabase::markMessagesRead(const std::string& chatId, int readerId) {
    for (auto& [id, message] : messages) {
        const bool delivered = message.status == Sent || message.status == Received;
        if (message.chatId == chatId && message.sender != readerId && delivered) {
            message.status = Read;
        }
    }
}
