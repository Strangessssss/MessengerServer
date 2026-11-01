#include <gtest/gtest.h>

#include <chrono>

#include "services/MessagesService/MessagesService.h"
#include "tests/support/ServiceFixture.h"

class MessagesServiceTests : public ServiceFixture {
    protected:
        void SetUp() override {
            alice = addUser("Alice");
            bob = addUser("Bob");
            carol = addUser("Carol");

            Chat newChat;
            newChat.userIds = {alice, bob};
            chat = chatsRepository.add(newChat);
        }

        MessagesService service{messagesRepository, chatsRepository};
        int alice = 0;
        int bob = 0;
        int carol = 0;
        Chat chat;
};

TEST_F(MessagesServiceTests, SendMessageStoresASentMessage) {
    const auto before = std::chrono::system_clock::now();

    const MessageResponse response = service.sendMessage(chat.id, alice, "Hello");

    ASSERT_TRUE(response.success);
    ASSERT_TRUE(response.response_data.has_value());
    EXPECT_FALSE(response.response_data->id.empty());
    EXPECT_EQ(response.response_data->chatId, chat.id);
    EXPECT_EQ(response.response_data->sender, alice);
    EXPECT_EQ(response.response_data->text, "Hello");
    EXPECT_EQ(response.response_data->status, Sent);
    EXPECT_GE(response.response_data->createdAt, before);

    const auto messages = messagesRepository.get_messages(chat.id);
    ASSERT_EQ(messages.size(), 1u);
    EXPECT_EQ(messages.front().id, response.response_data->id);
}

TEST_F(MessagesServiceTests, SendMessageNumbersMessagesFromOneInEachChat) {
    Chat newChat;
    newChat.userIds = {alice, carol};
    const Chat otherChat = chatsRepository.add(newChat);

    EXPECT_EQ(service.sendMessage(chat.id, alice, "first").response_data->seq, 1);
    EXPECT_EQ(service.sendMessage(chat.id, bob, "second").response_data->seq, 2);
    EXPECT_EQ(service.sendMessage(otherChat.id, alice, "other").response_data->seq, 1);
}

TEST_F(MessagesServiceTests, SendMessageRespondsNotFoundForUnknownChat) {
    EXPECT_TRUE(service.sendMessage("missing", alice, "Hello").not_found());
}

TEST_F(MessagesServiceTests, SendMessageRespondsNotFoundForNonMemberAndStoresNothing) {
    EXPECT_TRUE(service.sendMessage(chat.id, carol, "Hello").not_found());
    EXPECT_TRUE(messagesRepository.get_messages(chat.id).empty());
}

TEST_F(MessagesServiceTests, SendMessageRejectsEmptyTextAndStoresNothing) {
    const MessageResponse response = service.sendMessage(chat.id, alice, "");

    EXPECT_FALSE(response.success);
    EXPECT_FALSE(response.not_found());
    EXPECT_TRUE(messagesRepository.get_messages(chat.id).empty());
}

TEST_F(MessagesServiceTests, SendMessageHidesTheChatFromNonMembersEvenWithEmptyText) {
    EXPECT_TRUE(service.sendMessage(chat.id, carol, "").not_found());
}
