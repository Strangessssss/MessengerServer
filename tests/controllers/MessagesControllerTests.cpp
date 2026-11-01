#include <gtest/gtest.h>

#include <string>

#include "tests/support/ControllerFixture.h"

class MessagesControllerTests : public ChatRouteFixture {};

TEST_F(MessagesControllerTests, SendMessageRespondsCreatedWithTheMessage) {
    const std::string chatId = createChat();

    const crow::response response = send(HTTPMethod::POST, "/chats/" + chatId + "/messages", aliceToken, R"({"text":"Hello"})");

    EXPECT_EQ(response.code, 201);

    const crow::json::rvalue json = crow::json::load(response.body);
    ASSERT_TRUE(static_cast<bool>(json));
    EXPECT_FALSE(str(json["id"]).empty());
    EXPECT_EQ(json["seq"].i(), 1);
    EXPECT_EQ(str(json["chatId"]), chatId);
    EXPECT_EQ(json["senderId"].i(), alice);
    EXPECT_EQ(str(json["text"]), "Hello");
    EXPECT_EQ(str(json["status"]), "sent");
    EXPECT_GT(json["createdAt"].i(), 0);
}

TEST_F(MessagesControllerTests, SendMessageRejectsBadBodies) {
    const std::string url = "/chats/" + createChat() + "/messages";

    for (const std::string& body : {std::string(""), std::string("not json"), std::string("{}"), std::string(R"({"text":1})")}) {
        SCOPED_TRACE(body);
        expectError(send(HTTPMethod::POST, url, aliceToken, body), 400, "text is required");
    }
}

TEST_F(MessagesControllerTests, SendMessageRejectsEmptyText) {
    expectError(send(HTTPMethod::POST, "/chats/" + createChat() + "/messages", aliceToken, R"({"text":""})"), 400, "Message was rejected");
}

TEST_F(MessagesControllerTests, SendMessageRespondsNotFoundForUnknownChatAndNonMember) {
    const std::string chatId = createChat();

    expectError(send(HTTPMethod::POST, "/chats/missing/messages", aliceToken, R"({"text":"Hello"})"), 404, "Chat not found");
    expectError(send(HTTPMethod::POST, "/chats/" + chatId + "/messages", carolToken, R"({"text":"Hello"})"), 404, "Chat not found");
    EXPECT_TRUE(messagesRepository.get_messages(chatId).empty());
}
