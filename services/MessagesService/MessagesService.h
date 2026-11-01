#pragma once
#include <string>

#include "../../repositories/ChatsRepository/ChatsRepository.h"
#include "DTOs/responses/MessageResponse.h"
#include "../../repositories/MessagesRepository/MessagesRepository.h"

class MessagesService {
    public:
        MessagesService(MessagesRepository& messagesRepository, ChatsRepository& chatsRepository);

        // Responds with not found if the chat doesn't exist or the sender isn't a member,
        // and with a failure if the text is empty.
        MessageResponse sendMessage(std::string chatId, int senderId, std::string text);

    private:
        MessagesRepository& messagesRepository;
        ChatsRepository& chatsRepository;
};
