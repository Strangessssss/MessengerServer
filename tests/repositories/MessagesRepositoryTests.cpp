#include <gtest/gtest.h>

#include <string>

#include "database/MockDatabase/MockDatabase.h"
#include "repositories/MessagesRepository/MessagesRepository.h"

namespace {
    Message makeMessage(int seq, const std::string& chatId, int sender, MessageStatus status = Sent) {
        Message message;
        message.seq = seq;
        message.chatId = chatId;
        message.sender = sender;
        message.text = "message " + std::to_string(seq);
        message.status = status;
        return message;
    }
}

class MessagesRepositoryTests : public testing::Test {
    protected:
        MockDatabase database;
        MessagesRepository repository{database};
};

TEST_F(MessagesRepositoryTests, AddGeneratesAnIdAndStoresTheMessage) {
    const Message added = repository.add(makeMessage(1, "chat-1", 1));

    EXPECT_FALSE(added.id.empty());

    const auto messages = repository.get_messages("chat-1");
    ASSERT_EQ(messages.size(), 1u);
    EXPECT_EQ(messages.front().id, added.id);
    EXPECT_EQ(messages.front().text, added.text);
}

TEST_F(MessagesRepositoryTests, AddReplacesAnIdSetByTheCaller) {
    Message message = makeMessage(1, "chat-1", 1);
    message.id = "chosen-by-caller";

    EXPECT_NE(repository.add(message).id, "chosen-by-caller");
}

TEST_F(MessagesRepositoryTests, GetMessagesReturnsOnlyThatChatsMessagesInSeqOrder) {
    repository.add(makeMessage(2, "chat-1", 1));
    repository.add(makeMessage(1, "chat-1", 1));
    repository.add(makeMessage(1, "chat-2", 1));

    const auto messages = repository.get_messages("chat-1");

    ASSERT_EQ(messages.size(), 2u);
    EXPECT_EQ(messages[0].seq, 1);
    EXPECT_EQ(messages[1].seq, 2);
}

TEST_F(MessagesRepositoryTests, GetLatestMessageReturnsTheHighestSeq) {
    repository.add(makeMessage(2, "chat-1", 1));
    repository.add(makeMessage(3, "chat-1", 1));
    repository.add(makeMessage(1, "chat-1", 1));

    const auto latest = repository.get_latest_message("chat-1");

    ASSERT_TRUE(latest.has_value());
    EXPECT_EQ(latest->seq, 3);
}

TEST_F(MessagesRepositoryTests, GetLatestMessageReturnsNothingForEmptyChat) {
    EXPECT_FALSE(repository.get_latest_message("chat-1").has_value());
}

TEST_F(MessagesRepositoryTests, MarkReadChangesOnlyOtherUsersMessagesInThatChat) {
    repository.add(makeMessage(1, "chat-1", 2));
    repository.add(makeMessage(2, "chat-1", 1));
    repository.add(makeMessage(1, "chat-2", 2));

    repository.mark_read("chat-1", 1);

    const auto chat1 = repository.get_messages("chat-1");
    ASSERT_EQ(chat1.size(), 2u);
    EXPECT_EQ(chat1[0].status, Read);
    EXPECT_EQ(chat1[1].status, Sent);

    const auto chat2 = repository.get_messages("chat-2");
    ASSERT_EQ(chat2.size(), 1u);
    EXPECT_EQ(chat2[0].status, Sent);
}
