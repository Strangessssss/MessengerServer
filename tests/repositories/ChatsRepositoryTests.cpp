#include <gtest/gtest.h>

#include <algorithm>
#include <vector>

#include "database/MockDatabase/MockDatabase.h"
#include "repositories/ChatsRepository/ChatsRepository.h"

namespace {
    Chat makeChat(const std::vector<int>& userIds) {
        Chat chat;
        chat.userIds = userIds;
        return chat;
    }
}

class ChatsRepositoryTests : public testing::Test {
    protected:
        MockDatabase database;
        ChatsRepository repository{database};
};

TEST_F(ChatsRepositoryTests, AddGeneratesAnIdAndStoresTheChat) {
    const Chat added = repository.add(makeChat({1, 2}));

    EXPECT_FALSE(added.id.empty());

    const auto found = repository.get_chat(added.id);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->userIds, added.userIds);
}

TEST_F(ChatsRepositoryTests, AddReplacesAnIdSetByTheCaller) {
    Chat chat = makeChat({1, 2});
    chat.id = "chosen-by-caller";

    const Chat added = repository.add(chat);

    EXPECT_NE(added.id, "chosen-by-caller");
    EXPECT_FALSE(repository.get_chat("chosen-by-caller").has_value());
}

TEST_F(ChatsRepositoryTests, AddedChatsGetDifferentIds) {
    EXPECT_NE(repository.add(makeChat({1, 2})).id, repository.add(makeChat({1, 2})).id);
}

TEST_F(ChatsRepositoryTests, GetChatReturnsNothingForUnknownId) {
    EXPECT_FALSE(repository.get_chat("missing").has_value());
}

TEST_F(ChatsRepositoryTests, GetChatsReturnsOnlyTheUsersChats) {
    const Chat first = repository.add(makeChat({1, 2}));
    const Chat second = repository.add(makeChat({1, 3}));
    repository.add(makeChat({2, 3}));

    const auto chats = repository.get_chats(1);

    ASSERT_TRUE(chats.has_value());
    ASSERT_EQ(chats->size(), 2u);
    EXPECT_TRUE(std::ranges::find(*chats, first.id, &Chat::id) != chats->end());
    EXPECT_TRUE(std::ranges::find(*chats, second.id, &Chat::id) != chats->end());
}

TEST_F(ChatsRepositoryTests, GetChatsReturnsNothingForUserWithoutChats) {
    repository.add(makeChat({1, 2}));

    EXPECT_FALSE(repository.get_chats(3).has_value());
}

TEST_F(ChatsRepositoryTests, DeleteChatByIdRemovesOnlyThatChat) {
    const Chat deleted = repository.add(makeChat({1, 2}));
    const Chat kept = repository.add(makeChat({1, 2}));

    repository.delete_chat(deleted.id);

    EXPECT_FALSE(repository.get_chat(deleted.id).has_value());
    EXPECT_TRUE(repository.get_chat(kept.id).has_value());
}

TEST_F(ChatsRepositoryTests, DeleteChatByUserIdRemovesOnlyTheUsersChats) {
    const Chat first = repository.add(makeChat({1, 2}));
    const Chat second = repository.add(makeChat({1, 3}));
    const Chat kept = repository.add(makeChat({2, 3}));

    repository.delete_chat(1);

    EXPECT_FALSE(repository.get_chat(first.id).has_value());
    EXPECT_FALSE(repository.get_chat(second.id).has_value());
    EXPECT_TRUE(repository.get_chat(kept.id).has_value());
}

TEST_F(ChatsRepositoryTests, FindDirectChatWorksInEitherUserOrder) {
    const Chat chat = repository.add(makeChat({1, 2}));

    const auto forward = repository.find_direct_chat(1, 2);
    const auto backward = repository.find_direct_chat(2, 1);

    ASSERT_TRUE(forward.has_value());
    ASSERT_TRUE(backward.has_value());
    EXPECT_EQ(forward->id, chat.id);
    EXPECT_EQ(backward->id, chat.id);
}

TEST_F(ChatsRepositoryTests, FindDirectChatIgnoresGroupChats) {
    repository.add(makeChat({1, 2, 3}));

    EXPECT_FALSE(repository.find_direct_chat(1, 2).has_value());
}

TEST_F(ChatsRepositoryTests, FindDirectChatReturnsNothingForSameUserTwice) {
    repository.add(makeChat({1, 2}));

    EXPECT_FALSE(repository.find_direct_chat(1, 1).has_value());
}

TEST_F(ChatsRepositoryTests, FindDirectChatReturnsNothingWhenUsersShareNoChat) {
    repository.add(makeChat({1, 2}));
    repository.add(makeChat({3, 4}));

    EXPECT_FALSE(repository.find_direct_chat(1, 3).has_value());
}
