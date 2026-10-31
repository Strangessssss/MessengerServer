#include "ChatsService.h"

#include <algorithm>
#include <chrono>
#include <utility>

ChatsService::ChatsService(ChatsRepository& chatsRepository, MessagesRepository& messagesRepository, UsersRepository& usersRepository)
    : chatsRepository(chatsRepository), messagesRepository(messagesRepository), usersRepository(usersRepository) {}

ChatResponse ChatsService::addChat(std::vector<int> userIds) {
    std::ranges::sort(userIds);
    const auto duplicates = std::ranges::unique(userIds);
    userIds.erase(duplicates.begin(), duplicates.end());

    const bool allUsersExist = std::ranges::all_of(userIds, [this](int userId) {
        return usersRepository.findById(userId).has_value();
    });
    if (userIds.size() < 2 || !allUsersExist) {
        return ChatResponse(std::nullopt, false);
    }

    if (userIds.size() == 2) {
        const std::optional<Chat> existingChat = chatsRepository.find_direct_chat(userIds[0], userIds[1]);
        if (existingChat.has_value()) {
            return ChatResponse(existingChat);
        }
    }

    Chat chat;
    chat.userIds = std::move(userIds);
    chat.createdAt = std::chrono::system_clock::now();

    return ChatResponse(chatsRepository.add(std::move(chat)));
}

ChatSummaryListResponse ChatsService::getChats(int userId) const {
    const std::optional<std::vector<Chat>> chats = chatsRepository.get_chats(userId);
    if (!chats.has_value()) {
        return ChatSummaryListResponse(std::nullopt);
    }

    std::vector<ChatSummary> summaries;
    for (const Chat& chat : *chats) {
        ChatSummary summary;
        summary.chatId = chat.id;
        summary.partnerUsername = getPartnerUsername(chat, userId);

        const std::optional<Message> latest = messagesRepository.get_latest_message(chat.id);
        if (latest.has_value()) {
            summary.latestMessage = LatestMessage{latest->text, latest->createdAt, latest->status};
        }
        summaries.push_back(std::move(summary));
    }
    return ChatSummaryListResponse(std::move(summaries));
}

ChatResponse ChatsService::getChat(std::string chatId) const {
    return ChatResponse(chatsRepository.get_chat(std::move(chatId)));
}

ChatViewResponse ChatsService::getChatView(std::string chatId, int userId) {
    const std::optional<Chat> chat = chatsRepository.get_chat(chatId);
    if (!chat.has_value() || !chat->hasUser(userId)) {
        return ChatViewResponse(std::nullopt);
    }

    messagesRepository.mark_read(chatId, userId);

    ChatView view;
    view.chatId = chat->id;
    view.partnerUsername = getPartnerUsername(*chat, userId);
    view.messages = messagesRepository.get_messages(std::move(chatId));
    return ChatViewResponse(std::move(view));
}

ChatResponse ChatsService::deleteChat(std::string chatId, int userId) {
    std::optional<Chat> chat = chatsRepository.get_chat(chatId);
    if (!chat.has_value() || !chat->hasUser(userId)) {
        return ChatResponse(std::nullopt);
    }

    chatsRepository.delete_chat(std::move(chatId));
    return ChatResponse(std::move(chat));
}

ChatListResponse ChatsService::deleteChat(int userId) {
    std::optional<std::vector<Chat>> chats = chatsRepository.get_chats(userId);
    if (chats.has_value()) {
        chatsRepository.delete_chat(userId);
    }
    return ChatListResponse(std::move(chats));
}

std::string ChatsService::getPartnerUsername(const Chat& chat, int userId) const {
    for (const int memberId : chat.userIds) {
        if (memberId == userId) {
            continue;
        }
        const std::optional<User> user = usersRepository.findById(memberId);
        if (user.has_value()) {
            return user->username;
        }
    }
    return "";
}
