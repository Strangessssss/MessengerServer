#include "MessagesService.h"

#include <algorithm>
#include <chrono>
#include <utility>
#include <vector>

MessagesService::MessagesService(MessagesRepository& messagesRepository, ChatsRepository& chatsRepository, UsersRepository& usersRepository)
    : messagesRepository(messagesRepository), chatsRepository(chatsRepository), usersRepository(usersRepository) {}

MessageResponse MessagesService::sendMessage(std::string chatId, int senderId, std::string text) {
    const std::optional<Chat> chat = chatsRepository.get_chat(chatId);
    if (!chat.has_value() || !chat->hasUser(senderId)) {
        return MessageResponse(std::nullopt);
    }
    if (text.empty() || isBlocked(*chat, senderId)) {
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

MessageResponse MessagesService::sendMessage(int senderId, int recipientId, std::string text) {
    if (!usersRepository.findById(senderId).has_value() || !usersRepository.findById(recipientId).has_value()) {
        return MessageResponse(std::nullopt);
    }

    const std::optional<Chat> existingChat = chatsRepository.find_direct_chat(senderId, recipientId);
    if (existingChat.has_value()) {
        return sendMessage(existingChat->id, senderId, std::move(text));
    }

    Chat chat;
    chat.userIds = {senderId, recipientId};
    chat.createdAt = std::chrono::system_clock::now();

    if (senderId == recipientId || text.empty() || isBlocked(chat, senderId)) {
        return MessageResponse(std::nullopt, false);
    }

    const Chat createdChat = chatsRepository.add(std::move(chat));
    return sendMessage(createdChat.id, senderId, std::move(text));
}

MessageListResponse MessagesService::receiveMessages(std::string chatId, int userId, int afterSeq) {
    const std::optional<Chat> chat = chatsRepository.get_chat(chatId);
    if (!chat.has_value() || !chat->hasUser(userId)) {
        return MessageListResponse(std::nullopt);
    }

    messagesRepository.mark_received(chatId, userId);

    std::vector<Message> messages = messagesRepository.get_messages(std::move(chatId));
    std::erase_if(messages, [afterSeq](const Message& message) {
        return message.seq <= afterSeq;
    });
    return MessageListResponse(std::move(messages));
}

bool MessagesService::isBlocked(const Chat& chat, int senderId) const {
    for (const int memberId : chat.userIds) {
        const std::optional<User> member = usersRepository.findById(memberId);
        if (member.has_value() && std::ranges::find(member->blockedUserIds, senderId) != member->blockedUserIds.end()) {
            return true;
        }
    }
    return false;
}
