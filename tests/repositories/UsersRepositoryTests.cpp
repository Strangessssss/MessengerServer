#include <gtest/gtest.h>

#include <string>

#include "database/MockDatabase/MockDatabase.h"
#include "repositories/UsersRepository/UsersRepository.h"

namespace {
    User makeUser(const std::string& username) {
        User user;
        user.username = username;
        user.passwordHash = "passwordHash123";
        return user;
    }
}

class UsersRepositoryTests : public testing::Test {
    protected:
        MockDatabase database;
        UsersRepository repository{database};
};

TEST_F(UsersRepositoryTests, AddedUserCanBeFoundById) {
    const User added = repository.add(makeUser("Alice"));

    const auto found = repository.findById(added.id);

    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->username, "Alice");
    EXPECT_EQ(found->passwordHash, "passwordHash123");
}

TEST_F(UsersRepositoryTests, AddedUserCanBeFoundByUsername) {
    repository.add(makeUser("Alice"));
    const User added = repository.add(makeUser("Emanuel"));

    const auto found = repository.findByUsername("Emanuel");

    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->id, added.id);
}

TEST_F(UsersRepositoryTests, AddedUsersGetDifferentIds) {
    EXPECT_NE(repository.add(makeUser("Alice")).id, repository.add(makeUser("Emanuel")).id);
}

TEST_F(UsersRepositoryTests, FindReturnsNothingForUnknownUser) {
    EXPECT_FALSE(repository.findById(999).has_value());
    EXPECT_FALSE(repository.findByUsername("nobody").has_value());
}
