#include "MessagesService.h"

#include <chrono>
#include <utility>

MessagesService::MessagesService(MessagesRepository& messagesRepository, ChatsRepository& chatsRepository)
    : messagesRepository(messagesRepository), chatsRepository(chatsRepository) {}

MessageResponse MessagesService::sendMessage(std::string chatId, int senderId, std::string text) {
    const std::optional<Chat> chat = chatsRepository.get_chat(chatId);
    if (!chat.has_value() || !chat->hasUser(senderId)) {
        return MessageResponse(std::nullopt);
    }
    if (text.empty()) {
        return MessageResponse(std::nullopt, false);
    }

    const std::optional<Message> latest = messagesRepository.get_latest_message(chatId);

    Message message;
    message.seq = latest.has_value() ? latest->seq + 1 : 1;
    message.chatId = std::move(chatId);
    message.sender = senderId;
    message.text = std::move(text);
    message.status = Sent;
    message.createdAt = std::chrono::system_clock::now();

    return MessageResponse(messagesRepository.add(std::move(message)));
}
