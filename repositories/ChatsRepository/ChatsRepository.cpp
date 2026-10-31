#include "ChatsRepository.h"

#include <Guid.hpp>

ChatsRepository::ChatsRepository(Database& database) : database(database) {}

Chat ChatsRepository::add(Chat chat) {
    chat.id = xg::newGuid().str();

    database.insertChat(chat);
    return chat;
}

std::optional<std::vector<Chat>> ChatsRepository::get_chats(int userId) const {
    std::vector<Chat> userChats = database.findChatsByUserId(userId);

    if (userChats.empty()) {
        return std::nullopt;
    }
    return userChats;
}

void ChatsRepository::delete_chat(std::string chatId) {
    database.deleteChat(chatId);
}

void ChatsRepository::delete_chat(int userId) {
    for (const Chat& chat : database.findChatsByUserId(userId)) {
        database.deleteChat(chat.id);
    }
}

std::optional<Chat> ChatsRepository::get_chat(std::string chatId) const {
    return database.findChatById(chatId);
}

std::optional<Chat> ChatsRepository::find_direct_chat(int firstUserId, int secondUserId) const {
    if (firstUserId == secondUserId) {
        return std::nullopt;
    }
    for (const Chat& chat : database.findChatsByUserId(firstUserId)) {
        if (chat.userIds.size() == 2 && chat.hasUser(secondUserId)) {
            return chat;
        }
    }
    return std::nullopt;
}
