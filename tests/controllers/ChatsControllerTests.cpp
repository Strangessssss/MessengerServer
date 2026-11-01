#include <gtest/gtest.h>

#include <string>

#include "tests/support/ControllerFixture.h"

class ChatsControllerTests : public ChatRouteFixture {};

// GET /chats

TEST_F(ChatsControllerTests, GetChatsRespondsWithAnEmptyListForUserWithoutChats) {
    const crow::response response = send(HTTPMethod::GET, "/chats", aliceToken);

    EXPECT_EQ(response.code, 200);

    const crow::json::rvalue json = crow::json::load(response.body);
    ASSERT_TRUE(static_cast<bool>(json));
    EXPECT_EQ(json.t(), crow::json::type::List);
    EXPECT_EQ(json.size(), 0u);
}

TEST_F(ChatsControllerTests, GetChatsDescribesAChatWithoutMessages) {
    const std::string chatId = createChat();

    const crow::response response = send(HTTPMethod::GET, "/chats", aliceToken);

    EXPECT_EQ(response.code, 200);

    const crow::json::rvalue json = crow::json::load(response.body);
    ASSERT_TRUE(static_cast<bool>(json));
    ASSERT_EQ(json.size(), 1u);
    EXPECT_EQ(str(json[0]["chatId"]), chatId);
    EXPECT_EQ(str(json[0]["partnerUsername"]), "Bob");
    EXPECT_EQ(json[0]["unreadCount"].i(), 0);
    EXPECT_EQ(json[0]["latestMessage"].t(), crow::json::type::Null);
}

TEST_F(ChatsControllerTests, GetChatsDescribesTheLatestMessageAndUnreadCount) {
    const std::string chatId = createChat();
    send(HTTPMethod::POST, "/chats/" + chatId + "/messages", bobToken, R"({"text":"first"})");
    send(HTTPMethod::POST, "/chats/" + chatId + "/messages", bobToken, R"({"text":"second"})");

    const crow::response response = send(HTTPMethod::GET, "/chats", aliceToken);

    const crow::json::rvalue json = crow::json::load(response.body);
    ASSERT_TRUE(static_cast<bool>(json));
    ASSERT_EQ(json.size(), 1u);
    EXPECT_EQ(json[0]["unreadCount"].i(), 2);
    EXPECT_EQ(str(json[0]["latestMessage"]["text"]), "second");
    EXPECT_EQ(str(json[0]["latestMessage"]["status"]), "sent");
    EXPECT_GT(json[0]["latestMessage"]["createdAt"].i(), 0);
}

// POST /chats

TEST_F(ChatsControllerTests, CreateChatRespondsWithTheNewChat) {
    const crow::response response = send(HTTPMethod::POST, "/chats", aliceToken, R"({"username":"Bob"})");

    EXPECT_EQ(response.code, 200);

    const crow::json::rvalue json = crow::json::load(response.body);
    ASSERT_TRUE(static_cast<bool>(json));
    EXPECT_FALSE(str(json["id"]).empty());
    ASSERT_EQ(json["userIds"].size(), 2u);
    EXPECT_EQ(json["userIds"][0].i(), alice);
    EXPECT_EQ(json["userIds"][1].i(), bob);
    EXPECT_GT(json["createdAt"].i(), 0);
}

TEST_F(ChatsControllerTests, CreateChatReturnsTheExistingChatFromEitherUser) {
    const std::string chatId = createChat();

    const crow::response response = send(HTTPMethod::POST, "/chats", bobToken, R"({"username":"Alice"})");

    EXPECT_EQ(response.code, 200);
    EXPECT_EQ(str(crow::json::load(response.body)["id"]), chatId);
}

TEST_F(ChatsControllerTests, CreateChatRejectsBadBodies) {
    for (const std::string& body : {std::string(""), std::string("not json"), std::string("{}"), std::string(R"({"username":1})")}) {
        SCOPED_TRACE(body);
        expectError(send(HTTPMethod::POST, "/chats", aliceToken, body), 400, "Username missing");
    }
}

TEST_F(ChatsControllerTests, CreateChatRespondsNotFoundForUnknownUsername) {
    expectError(send(HTTPMethod::POST, "/chats", aliceToken, R"({"username":"nobody"})"), 404, "User not found");
}

TEST_F(ChatsControllerTests, CreateChatRejectsChatWithYourself) {
    expectError(send(HTTPMethod::POST, "/chats", aliceToken, R"({"username":"Alice"})"), 400, "You can't create a chat with yourself");
}

// GET /chats/<id>

TEST_F(ChatsControllerTests, GetChatViewRespondsWithMessagesAndMarksThemRead) {
    const std::string chatId = createChat();
    send(HTTPMethod::POST, "/chats/" + chatId + "/messages", bobToken, R"({"text":"first"})");
    send(HTTPMethod::POST, "/chats/" + chatId + "/messages", aliceToken, R"({"text":"second"})");

    const crow::response response = send(HTTPMethod::GET, "/chats/" + chatId, aliceToken);

    EXPECT_EQ(response.code, 200);

    const crow::json::rvalue json = crow::json::load(response.body);
    ASSERT_TRUE(static_cast<bool>(json));
    EXPECT_EQ(str(json["chatId"]), chatId);
    EXPECT_EQ(str(json["partnerUsername"]), "Bob");
    ASSERT_EQ(json["messages"].size(), 2u);
    EXPECT_EQ(str(json["messages"][0]["text"]), "first");
    EXPECT_EQ(str(json["messages"][0]["status"]), "read");
    EXPECT_EQ(str(json["messages"][1]["text"]), "second");
    EXPECT_EQ(str(json["messages"][1]["status"]), "sent");
}

TEST_F(ChatsControllerTests, GetChatViewRespondsWithAnEmptyMessageList) {
    const crow::response response = send(HTTPMethod::GET, "/chats/" + createChat(), aliceToken);

    EXPECT_EQ(response.code, 200);

    const crow::json::rvalue json = crow::json::load(response.body);
    ASSERT_TRUE(static_cast<bool>(json));
    EXPECT_EQ(json["messages"].t(), crow::json::type::List);
    EXPECT_EQ(json["messages"].size(), 0u);
}

TEST_F(ChatsControllerTests, GetChatViewRespondsNotFoundForUnknownChatAndNonMember) {
    const std::string chatId = createChat();

    expectError(send(HTTPMethod::GET, "/chats/missing", aliceToken), 404, "Chat not found");
    expectError(send(HTTPMethod::GET, "/chats/" + chatId, carolToken), 404, "Chat not found");
}

// DELETE /chats/<id>

TEST_F(ChatsControllerTests, DeleteChatRespondsNoContentAndRemovesTheChat) {
    const std::string chatId = createChat();

    const crow::response response = send(HTTPMethod::Delete, "/chats/" + chatId, aliceToken);

    EXPECT_EQ(response.code, 204);
    EXPECT_TRUE(response.body.empty());
    expectError(send(HTTPMethod::GET, "/chats/" + chatId, aliceToken), 404, "Chat not found");
}

TEST_F(ChatsControllerTests, DeleteChatRespondsNotFoundForUnknownChat) {
    expectError(send(HTTPMethod::Delete, "/chats/missing", aliceToken), 404, "Chat not found");
}

TEST_F(ChatsControllerTests, DeleteChatKeepsTheChatWhenTheUserIsNotAMember) {
    const std::string chatId = createChat();

    expectError(send(HTTPMethod::Delete, "/chats/" + chatId, carolToken), 404, "Chat not found");
    EXPECT_EQ(send(HTTPMethod::GET, "/chats/" + chatId, aliceToken).code, 200);
}
