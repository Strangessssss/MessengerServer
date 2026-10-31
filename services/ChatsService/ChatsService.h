#pragma once
#include <string>
#include <vector>

#include "../../repositories/ChatsRepository/ChatsRepository.h"
#include "../../repositories/ChatsRepository/DTOs/responses/ChatListResponse.h"
#include "../../repositories/ChatsRepository/DTOs/responses/ChatResponse.h"
#include "../../repositories/ChatsRepository/DTOs/responses/ChatSummaryListResponse.h"
#include "../../repositories/ChatsRepository/DTOs/responses/ChatViewResponse.h"
#include "../../repositories/MessagesRepository/MessagesRepository.h"
#include "../../repositories/UsersRepository/UsersRepository.h"

class ChatsService {
    public:
        ChatsService(ChatsRepository& chatsRepository, MessagesRepository& messagesRepository, UsersRepository& usersRepository);

        // Responds with a failure unless there are at least two distinct, existing users.
        // With two users, responds with their existing direct chat if there is one.
        ChatResponse addChat(std::vector<int> userIds);

        // Does not mark anything as read.
        ChatSummaryListResponse getChats(int userId) const;
        ChatResponse getChat(std::string chatId) const;

        // Marks the messages other members sent in this chat as read.
        // Responds with not found if the chat doesn't exist or the user isn't a member.
        ChatViewResponse getChatView(std::string chatId, int userId);

        // Responds with the deleted chat, or not found if it doesn't exist or the user isn't a member.
        ChatResponse deleteChat(std::string chatId, int userId);

        // Responds with the deleted chats, or not found if there were none.
        ChatListResponse deleteChat(int userId);

    private:
        std::string getPartnerUsername(const Chat& chat, int userId) const;

        ChatsRepository& chatsRepository;
        MessagesRepository& messagesRepository;
        UsersRepository& usersRepository;
};
