#pragma once
#include <gtest/gtest.h>

#include <string>

#include "database/MockDatabase/MockDatabase.h"
#include "repositories/ChatsRepository/ChatsRepository.h"
#include "repositories/MessagesRepository/MessagesRepository.h"
#include "repositories/UsersRepository/UsersRepository.h"

// Real repositories over an in-memory database, for testing the services above them.
class ServiceFixture : public testing::Test {
    protected:
        int addUser(const std::string& username) {
            User user;
            user.username = username;
            user.passwordHash = "passwordHash123";
            return usersRepository.add(user).id;
        }

        MockDatabase database;
        UsersRepository usersRepository{database};
        ChatsRepository chatsRepository{database};
        MessagesRepository messagesRepository{database};
};
