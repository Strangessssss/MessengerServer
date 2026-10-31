#pragma once
#include <string>

#include "../../repositories/ChatsRepository/ChatsRepository.h"
#include "DTOs/responses/MessageListResponse.h"
#include "DTOs/responses/MessageResponse.h"
#include "../../repositories/MessagesRepository/MessagesRepository.h"
#include "../../repositories/UsersRepository/UsersRepository.h"

class MessagesService {
    public:
        MessagesService(MessagesRepository& messagesRepository, ChatsRepository& chatsRepository, UsersRepository& usersRepository);

        // Responds with not found if the chat doesn't exist or the sender isn't a member,
        // and with a failure if the text is empty or a member has blocked the sender.
        MessageResponse sendMessage(std::string chatId, int senderId, std::string text);

        // Sends to the direct chat between the two users, creating it if it doesn't exist.
        // Responds with not found if either user doesn't exist, and with a failure if the text
        // is empty, the users are the same or the recipient has blocked the sender. A rejected
        // message never creates a chat.
        MessageResponse sendMessage(int senderId, int recipientId, std::string text);

        // Responds with the messages after afterSeq and marks the ones other members sent as received.
        // Responds with not found if the chat doesn't exist or the user isn't a member.
        MessageListResponse receiveMessages(std::string chatId, int userId, int afterSeq);

    private:
        bool isBlocked(const Chat& chat, int senderId) const;

        MessagesRepository& messagesRepository;
        ChatsRepository& chatsRepository;
        UsersRepository& usersRepository;
};
