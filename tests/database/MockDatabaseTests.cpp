
#include <gtest/gtest.h>

#include <algorithm>
#include <optional>
#include <string>
#include <vector>

#include "database/Database/Database.h"
#include "database/MockDatabase/MockDatabase.h"

TEST(MockDatabaseTests, InsertedUserCanBeFoundById) {
    MockDatabase database;

    User newUser;

    newUser.username = "alice";
    newUser.passwordHash = "passwordHash123";


    const User insertedUser = database.insertUser(newUser);
    const auto foundUser = database.findUserById(insertedUser.id);

    ASSERT_TRUE(foundUser != std::nullopt);
    ASSERT_TRUE(foundUser->username == newUser.username);
    ASSERT_TRUE(foundUser->passwordHash == newUser.passwordHash);
}

TEST(MockDatabaseTests, InsertedUserCanBeFoundByUsername) {
    MockDatabase database;

    User newUser;

    newUser.username = "alice";
    newUser.passwordHash = "passwordHash123";


    const User insertedUser = database.insertUser(newUser);
    const auto foundUser = database.findUserByUsername(insertedUser.username);

    ASSERT_TRUE(foundUser != std::nullopt);
    ASSERT_TRUE(foundUser->username == newUser.username);
    ASSERT_TRUE(foundUser->passwordHash == newUser.passwordHash);
}

TEST(MockDatabaseTests, InsertedChatCanBeFoundById) {
    MockDatabase database;

    User newUser1;
    newUser1.username = "Alice";
    newUser1.passwordHash = "passwordHash123";

    User newUser2;
    newUser2.username = "Emanuel";
    newUser2.passwordHash = "passwordHash123";

    const User insertedUser1 = database.insertUser(newUser1);
    const User insertedUser2 = database.insertUser(newUser2);

    Chat newChat;
    newChat.id = "chat-1";
    newChat.userIds.push_back(insertedUser1.id);
    newChat.userIds.push_back(insertedUser2.id);

    ASSERT_TRUE(database.insertChat(newChat).has_value());
    EXPECT_FALSE(database.insertChat(newChat).has_value());

    const auto foundChat = database.findChatById(newChat.id);
    ASSERT_TRUE(foundChat.has_value());
    EXPECT_EQ(foundChat->id, newChat.id);
    EXPECT_EQ(foundChat->userIds, newChat.userIds);
}

TEST(MockDatabaseTests, InsertedChatCanBeFoundByUserId) {
    MockDatabase database;

    User newUser1;
    newUser1.username = "Alice";
    newUser1.passwordHash = "passwordHash123";

    User newUser2;
    newUser2.username = "Emanuel";
    newUser2.passwordHash = "passwordHash123";

    User newUser3;
    newUser3.username = "Alex";
    newUser3.passwordHash = "passwordHash123";

    const User insertedUser1 = database.insertUser(newUser1);
    const User insertedUser2 = database.insertUser(newUser2);
    const User insertedUser3 = database.insertUser(newUser3);

    Chat newChat1;
    newChat1.id = "chat-1";
    newChat1.userIds.push_back(insertedUser1.id);
    newChat1.userIds.push_back(insertedUser2.id);
    ASSERT_TRUE(database.insertChat(newChat1).has_value());

    Chat newChat2;
    newChat2.id = "chat-2";
    newChat2.userIds.push_back(insertedUser2.id);
    newChat2.userIds.push_back(insertedUser3.id);
    ASSERT_TRUE(database.insertChat(newChat2).has_value());

    const auto foundChats1 = database.findChatsByUserId(insertedUser1.id);
    EXPECT_EQ(foundChats1.front().id, newChat1.id);
    EXPECT_EQ(foundChats1.front().userIds, newChat1.userIds);
}

namespace {
    Message makeMessage(const std::string& id, int seq, const std::string& chatId, int sender, MessageStatus status) {
        Message message;
        message.id = id;
        message.seq = seq;
        message.chatId = chatId;
        message.sender = sender;
        message.text = "text of " + id;
        message.status = status;
        return message;
    }

    Chat makeChat(const std::string& id, const std::vector<int>& userIds) {
        Chat chat;
        chat.id = id;
        chat.userIds = userIds;
        return chat;
    }

    std::optional<Message> findMessage(const std::vector<Message>& messages, const std::string& id) {
        const auto it = std::ranges::find(messages, id, &Message::id);
        if (it == messages.end()) {
            return std::nullopt;
        }
        return *it;
    }
}

TEST(MockDatabaseTests, InsertedUsersGetDifferentIds) {
    MockDatabase database;

    User newUser1;
    newUser1.username = "Alice";

    User newUser2;
    newUser2.username = "Emanuel";

    const User insertedUser1 = database.insertUser(newUser1);
    const User insertedUser2 = database.insertUser(newUser2);

    EXPECT_NE(insertedUser1.id, insertedUser2.id);
}

TEST(MockDatabaseTests, FindUserByIdReturnsNulloptForUnknownId) {
    MockDatabase database;

    EXPECT_FALSE(database.findUserById(999).has_value());
}

TEST(MockDatabaseTests, FindUserByUsernameReturnsTheMatchingUser) {
    MockDatabase database;

    User newUser1;
    newUser1.username = "Alice";

    User newUser2;
    newUser2.username = "Emanuel";

    database.insertUser(newUser1);
    const User insertedUser2 = database.insertUser(newUser2);

    const auto foundUser = database.findUserByUsername("Emanuel");
    ASSERT_TRUE(foundUser.has_value());
    EXPECT_EQ(foundUser->id, insertedUser2.id);
}

TEST(MockDatabaseTests, FindUserByUsernameReturnsNulloptForUnknownUsername) {
    MockDatabase database;

    EXPECT_FALSE(database.findUserByUsername("nobody").has_value());
}

TEST(MockDatabaseTests, FindChatByIdReturnsNulloptForUnknownId) {
    MockDatabase database;

    EXPECT_FALSE(database.findChatById("missing").has_value());
}

TEST(MockDatabaseTests, FindChatsByUserIdReturnsEveryChatOfTheUser) {
    MockDatabase database;

    ASSERT_TRUE(database.insertChat(makeChat("chat-1", {1, 2})).has_value());
    ASSERT_TRUE(database.insertChat(makeChat("chat-2", {1, 3})).has_value());
    ASSERT_TRUE(database.insertChat(makeChat("chat-3", {2, 3})).has_value());

    const auto foundChats = database.findChatsByUserId(1);

    ASSERT_EQ(foundChats.size(), 2u);
    EXPECT_TRUE(std::ranges::find(foundChats, "chat-1", &Chat::id) != foundChats.end());
    EXPECT_TRUE(std::ranges::find(foundChats, "chat-2", &Chat::id) != foundChats.end());
}

TEST(MockDatabaseTests, FindChatsByUserIdReturnsEmptyForUserWithoutChats) {
    MockDatabase database;

    ASSERT_TRUE(database.insertChat(makeChat("chat-1", {1, 2})).has_value());

    EXPECT_TRUE(database.findChatsByUserId(3).empty());
}

TEST(MockDatabaseTests, DeleteChatRemovesTheChatAndItsMessages) {
    MockDatabase database;

    ASSERT_TRUE(database.insertChat(makeChat("chat-1", {1, 2})).has_value());
    database.insertMessage(makeMessage("message-1", 1, "chat-1", 1, Sent));

    database.deleteChat("chat-1");

    EXPECT_FALSE(database.findChatById("chat-1").has_value());
    EXPECT_TRUE(database.findMessagesByChatId("chat-1").empty());
}

TEST(MockDatabaseTests, DeleteChatKeepsOtherChatsAndTheirMessages) {
    MockDatabase database;

    ASSERT_TRUE(database.insertChat(makeChat("chat-1", {1, 2})).has_value());
    ASSERT_TRUE(database.insertChat(makeChat("chat-2", {1, 2})).has_value());
    database.insertMessage(makeMessage("message-1", 1, "chat-1", 1, Sent));
    database.insertMessage(makeMessage("message-2", 1, "chat-2", 1, Sent));

    database.deleteChat("chat-1");

    EXPECT_TRUE(database.findChatById("chat-2").has_value());

    const auto foundMessages = database.findMessagesByChatId("chat-2");
    ASSERT_EQ(foundMessages.size(), 1u);
    EXPECT_EQ(foundMessages.front().id, "message-2");
}

TEST(MockDatabaseTests, DeleteChatIgnoresUnknownId) {
    MockDatabase database;

    ASSERT_TRUE(database.insertChat(makeChat("chat-1", {1, 2})).has_value());

    database.deleteChat("missing");

    EXPECT_TRUE(database.findChatById("chat-1").has_value());
}

TEST(MockDatabaseTests, InsertedMessageCanBeFoundByChatId) {
    MockDatabase database;

    const Message newMessage = makeMessage("message-1", 1, "chat-1", 1, Sent);
    database.insertMessage(newMessage);

    const auto foundMessages = database.findMessagesByChatId("chat-1");

    ASSERT_EQ(foundMessages.size(), 1u);
    EXPECT_EQ(foundMessages.front().id, newMessage.id);
    EXPECT_EQ(foundMessages.front().seq, newMessage.seq);
    EXPECT_EQ(foundMessages.front().chatId, newMessage.chatId);
    EXPECT_EQ(foundMessages.front().sender, newMessage.sender);
    EXPECT_EQ(foundMessages.front().text, newMessage.text);
    EXPECT_EQ(foundMessages.front().status, newMessage.status);
}

TEST(MockDatabaseTests, InsertMessageKeepsTheOriginalWhenIdAlreadyExists) {
    MockDatabase database;

    database.insertMessage(makeMessage("message-1", 1, "chat-1", 1, Sent));
    database.insertMessage(makeMessage("message-1", 2, "chat-1", 2, Read));

    const auto foundMessages = database.findMessagesByChatId("chat-1");

    ASSERT_EQ(foundMessages.size(), 1u);
    EXPECT_EQ(foundMessages.front().seq, 1);
    EXPECT_EQ(foundMessages.front().sender, 1);
}

TEST(MockDatabaseTests, FindMessagesByChatIdReturnsOnlyThatChatsMessages) {
    MockDatabase database;

    database.insertMessage(makeMessage("message-1", 1, "chat-1", 1, Sent));
    database.insertMessage(makeMessage("message-2", 1, "chat-2", 1, Sent));

    const auto foundMessages = database.findMessagesByChatId("chat-1");

    ASSERT_EQ(foundMessages.size(), 1u);
    EXPECT_EQ(foundMessages.front().id, "message-1");
}

TEST(MockDatabaseTests, FindMessagesByChatIdSortsBySeq) {
    MockDatabase database;

    database.insertMessage(makeMessage("message-3", 3, "chat-1", 1, Sent));
    database.insertMessage(makeMessage("message-1", 1, "chat-1", 1, Sent));
    database.insertMessage(makeMessage("message-2", 2, "chat-1", 1, Sent));

    const auto foundMessages = database.findMessagesByChatId("chat-1");

    ASSERT_EQ(foundMessages.size(), 3u);
    EXPECT_EQ(foundMessages[0].seq, 1);
    EXPECT_EQ(foundMessages[1].seq, 2);
    EXPECT_EQ(foundMessages[2].seq, 3);
}

TEST(MockDatabaseTests, FindMessagesByChatIdReturnsEmptyForChatWithoutMessages) {
    MockDatabase database;

    EXPECT_TRUE(database.findMessagesByChatId("chat-1").empty());
}

TEST(MockDatabaseTests, MarkMessagesReadMarksOtherUsersSentMessages) {
    MockDatabase database;

    database.insertMessage(makeMessage("message-1", 1, "chat-1", 2, Sent));

    database.markMessagesRead("chat-1", 1);

    const auto message = findMessage(database.findMessagesByChatId("chat-1"), "message-1");
    ASSERT_TRUE(message.has_value());
    EXPECT_EQ(message->status, Read);
}

TEST(MockDatabaseTests, MarkMessagesReadSkipsTheReadersOwnMessages) {
    MockDatabase database;

    database.insertMessage(makeMessage("message-1", 1, "chat-1", 1, Sent));

    database.markMessagesRead("chat-1", 1);

    const auto message = findMessage(database.findMessagesByChatId("chat-1"), "message-1");
    ASSERT_TRUE(message.has_value());
    EXPECT_EQ(message->status, Sent);
}

TEST(MockDatabaseTests, MarkMessagesReadOnlyChangesSentMessages) {
    MockDatabase database;

    database.insertMessage(makeMessage("pending", 1, "chat-1", 2, Pending));
    database.insertMessage(makeMessage("error", 2, "chat-1", 2, Error));

    database.markMessagesRead("chat-1", 1);

    const auto foundMessages = database.findMessagesByChatId("chat-1");
    const auto pending = findMessage(foundMessages, "pending");
    const auto error = findMessage(foundMessages, "error");
    ASSERT_TRUE(pending.has_value());
    ASSERT_TRUE(error.has_value());
    EXPECT_EQ(pending->status, Pending);
    EXPECT_EQ(error->status, Error);
}

TEST(MockDatabaseTests, MarkMessagesReadLeavesOtherChatsUnchanged) {
    MockDatabase database;

    database.insertMessage(makeMessage("message-1", 1, "chat-2", 2, Sent));

    database.markMessagesRead("chat-1", 1);

    const auto message = findMessage(database.findMessagesByChatId("chat-2"), "message-1");
    ASSERT_TRUE(message.has_value());
    EXPECT_EQ(message->status, Sent);
}