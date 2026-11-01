#include "MessagesRepository.h"

#include <Guid.hpp>

MessagesRepository::MessagesRepository(Database& database) : database(database) {}

Message MessagesRepository::add(Message message) {
    message.id = xg::newGuid().str();

    database.insertMessage(message);
    return message;
}

std::vector<Message> MessagesRepository::get_messages(std::string chatId) const {
    return database.findMessagesByChatId(chatId);
}

std::optional<Message> MessagesRepository::get_latest_message(std::string chatId) const {
    const std::vector<Message> messages = database.findMessagesByChatId(chatId);
    if (messages.empty()) {
        return std::nullopt;
    }
    return messages.back();
}

void MessagesRepository::mark_read(std::string chatId, int readerId) {
    database.markMessagesRead(chatId, readerId);
}
