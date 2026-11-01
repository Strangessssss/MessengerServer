#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <string>
#include <vector>

#include "services/ChatsService/ChatsService.h"
#include "tests/support/ServiceFixture.h"

class ChatsServiceTests : public ServiceFixture {
    protected:
        void SetUp() override {
            alice = addUser("Alice");
            bob = addUser("Bob");
            carol = addUser("Carol");
        }

        Chat addChat(const std::vector<int>& userIds) {
            return *service.addChat(userIds).response_data;
        }

        void addMessage(const std::string& chatId, int seq, int sender, MessageStatus status = Sent) {
            Message message;
            message.seq = seq;
            message.chatId = chatId;
            message.sender = sender;
            message.text = "message " + std::to_string(seq);
            message.status = status;
            messagesRepository.add(message);
        }

        ChatsService service{chatsRepository, messagesRepository, usersRepository};
        int alice = 0;
        int bob = 0;
        int carol = 0;
};

TEST_F(ChatsServiceTests, AddChatCreatesAChatWithSortedUniqueMembers) {
    const ChatResponse response = service.addChat({bob, alice, bob});

    ASSERT_TRUE(response.success);
    ASSERT_TRUE(response.response_data.has_value());
    EXPECT_FALSE(response.response_data->id.empty());
    EXPECT_EQ(response.response_data->userIds, (std::vector<int>{alice, bob}));
    EXPECT_TRUE(chatsRepository.get_chat(response.response_data->id).has_value());
}

TEST_F(ChatsServiceTests, AddChatSetsCreatedAt) {
    const auto before = std::chrono::system_clock::now();

    const Chat chat = addChat({alice, bob});

    EXPECT_GE(chat.createdAt, before);
    EXPECT_LE(chat.createdAt, std::chrono::system_clock::now());
}

TEST_F(ChatsServiceTests, AddChatRejectsFewerThanTwoDistinctUsers) {
    EXPECT_FALSE(service.addChat({}).success);
    EXPECT_FALSE(service.addChat({alice}).success);
    EXPECT_FALSE(service.addChat({alice, alice}).success);
}

TEST_F(ChatsServiceTests, AddChatRejectsUnknownUser) {
    const ChatResponse response = service.addChat({alice, 999});

    EXPECT_FALSE(response.success);
    EXPECT_FALSE(chatsRepository.get_chats(alice).has_value());
}

TEST_F(ChatsServiceTests, AddChatReturnsTheExistingDirectChatInEitherOrder) {
    const Chat chat = addChat({alice, bob});

    const ChatResponse response = service.addChat({bob, alice});

    ASSERT_TRUE(response.response_data.has_value());
    EXPECT_EQ(response.response_data->id, chat.id);
    EXPECT_EQ(chatsRepository.get_chats(alice)->size(), 1u);
}

TEST_F(ChatsServiceTests, AddChatCreatesANewGroupChatEachTime) {
    const Chat first = addChat({alice, bob, carol});
    const Chat second = addChat({alice, bob, carol});

    EXPECT_NE(first.id, second.id);
}

TEST_F(ChatsServiceTests, AddChatDoesNotReuseAGroupChatAsADirectChat) {
    const Chat group = addChat({alice, bob, carol});

    EXPECT_NE(addChat({alice, bob}).id, group.id);
}

TEST_F(ChatsServiceTests, GetChatsRespondsNotFoundForUserWithoutChats) {
    EXPECT_TRUE(service.getChats(alice).not_found());
}

TEST_F(ChatsServiceTests, GetChatsSummarizesAChatWithoutMessages) {
    const Chat chat = addChat({alice, bob});

    const ChatSummaryListResponse response = service.getChats(alice);

    ASSERT_TRUE(response.response_data.has_value());
    ASSERT_EQ(response.response_data->size(), 1u);
    const ChatSummary& summary = response.response_data->front();
    EXPECT_EQ(summary.chatId, chat.id);
    EXPECT_EQ(summary.partnerUsername, "Bob");
    EXPECT_FALSE(summary.latestMessage.has_value());
    EXPECT_EQ(summary.unreadCount, 0);
}

TEST_F(ChatsServiceTests, GetChatsReportsThePartnerFromEachUsersSide) {
    addChat({alice, bob});

    EXPECT_EQ(service.getChats(alice).response_data->front().partnerUsername, "Bob");
    EXPECT_EQ(service.getChats(bob).response_data->front().partnerUsername, "Alice");
}

TEST_F(ChatsServiceTests, GetChatsReportsTheLatestMessage) {
    const Chat chat = addChat({alice, bob});
    addMessage(chat.id, 2, bob);
    addMessage(chat.id, 1, alice);

    const ChatSummaryListResponse response = service.getChats(alice);

    const auto& latest = response.response_data->front().latestMessage;
    ASSERT_TRUE(latest.has_value());
    EXPECT_EQ(latest->text, "message 2");
    EXPECT_EQ(latest->status, Sent);
}

TEST_F(ChatsServiceTests, GetChatsCountsOnlyOtherUsersUnreadMessages) {
    const Chat chat = addChat({alice, bob});
    addMessage(chat.id, 1, bob, Read);
    addMessage(chat.id, 2, bob, Sent);
    addMessage(chat.id, 3, bob, Sent);
    addMessage(chat.id, 4, alice, Sent);

    EXPECT_EQ(service.getChats(alice).response_data->front().unreadCount, 2);
    EXPECT_EQ(service.getChats(bob).response_data->front().unreadCount, 1);
}

TEST_F(ChatsServiceTests, GetChatsDoesNotMarkAnythingAsRead) {
    const Chat chat = addChat({alice, bob});
    addMessage(chat.id, 1, bob);

    service.getChats(alice);

    EXPECT_EQ(messagesRepository.get_messages(chat.id).front().status, Sent);
}

TEST_F(ChatsServiceTests, GetChatReturnsTheChat) {
    const Chat chat = addChat({alice, bob});

    const ChatResponse response = service.getChat(chat.id);

    ASSERT_TRUE(response.response_data.has_value());
    EXPECT_EQ(response.response_data->id, chat.id);
}

TEST_F(ChatsServiceTests, GetChatRespondsNotFoundForUnknownId) {
    EXPECT_TRUE(service.getChat("missing").not_found());
}

TEST_F(ChatsServiceTests, GetChatViewReturnsThePartnerAndOrderedMessages) {
    const Chat chat = addChat({alice, bob});
    addMessage(chat.id, 2, alice);
    addMessage(chat.id, 1, bob);

    const ChatViewResponse response = service.getChatView(chat.id, alice);

    ASSERT_TRUE(response.response_data.has_value());
    EXPECT_EQ(response.response_data->chatId, chat.id);
    EXPECT_EQ(response.response_data->partnerUsername, "Bob");
    ASSERT_EQ(response.response_data->messages.size(), 2u);
    EXPECT_EQ(response.response_data->messages[0].seq, 1);
    EXPECT_EQ(response.response_data->messages[1].seq, 2);
}

TEST_F(ChatsServiceTests, GetChatViewMarksOnlyOtherUsersMessagesAsRead) {
    const Chat chat = addChat({alice, bob});
    addMessage(chat.id, 1, bob);
    addMessage(chat.id, 2, alice);

    const ChatViewResponse response = service.getChatView(chat.id, alice);

    ASSERT_TRUE(response.response_data.has_value());
    ASSERT_EQ(response.response_data->messages.size(), 2u);
    EXPECT_EQ(response.response_data->messages[0].status, Read);
    EXPECT_EQ(response.response_data->messages[1].status, Sent);
}

TEST_F(ChatsServiceTests, GetChatViewRespondsNotFoundForUnknownChat) {
    EXPECT_TRUE(service.getChatView("missing", alice).not_found());
}

TEST_F(ChatsServiceTests, GetChatViewHidesTheChatFromNonMembersWithoutMarkingRead) {
    const Chat chat = addChat({alice, bob});
    addMessage(chat.id, 1, bob);

    EXPECT_TRUE(service.getChatView(chat.id, carol).not_found());
    EXPECT_EQ(messagesRepository.get_messages(chat.id).front().status, Sent);
}

TEST_F(ChatsServiceTests, DeleteChatReturnsTheChatAndRemovesItWithItsMessages) {
    const Chat chat = addChat({alice, bob});
    addMessage(chat.id, 1, bob);

    const ChatResponse response = service.deleteChat(chat.id, alice);

    ASSERT_TRUE(response.response_data.has_value());
    EXPECT_EQ(response.response_data->id, chat.id);
    EXPECT_FALSE(chatsRepository.get_chat(chat.id).has_value());
    EXPECT_TRUE(messagesRepository.get_messages(chat.id).empty());
}

TEST_F(ChatsServiceTests, DeleteChatRespondsNotFoundForUnknownChat) {
    EXPECT_TRUE(service.deleteChat("missing", alice).not_found());
}

TEST_F(ChatsServiceTests, DeleteChatKeepsTheChatWhenTheUserIsNotAMember) {
    const Chat chat = addChat({alice, bob});

    EXPECT_TRUE(service.deleteChat(chat.id, carol).not_found());
    EXPECT_TRUE(chatsRepository.get_chat(chat.id).has_value());
}

TEST_F(ChatsServiceTests, DeleteAllChatsReturnsAndRemovesOnlyTheUsersChats) {
    const Chat first = addChat({alice, bob});
    const Chat second = addChat({alice, carol});
    const Chat kept = addChat({bob, carol});

    const ChatListResponse response = service.deleteChat(alice);

    ASSERT_TRUE(response.response_data.has_value());
    ASSERT_EQ(response.response_data->size(), 2u);
    EXPECT_TRUE(std::ranges::find(*response.response_data, first.id, &Chat::id) != response.response_data->end());
    EXPECT_TRUE(std::ranges::find(*response.response_data, second.id, &Chat::id) != response.response_data->end());
    EXPECT_FALSE(chatsRepository.get_chats(alice).has_value());
    EXPECT_TRUE(chatsRepository.get_chat(kept.id).has_value());
}

TEST_F(ChatsServiceTests, DeleteAllChatsRespondsNotFoundForUserWithoutChats) {
    EXPECT_TRUE(service.deleteChat(alice).not_found());
}
