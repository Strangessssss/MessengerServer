#include <gtest/gtest.h>

#include <string>

#include "services/UsersService/UsersService.h"
#include "tests/support/ServiceFixture.h"

class UsersServiceTests : public ServiceFixture {
    protected:
        UsersService service{usersRepository};
};

TEST_F(UsersServiceTests, GetUserIdByUsernameReturnsTheId) {
    addUser("Alice");
    const int id = addUser("Emanuel");

    const UserIdResponse response = service.getUserIdByUsername("Emanuel");

    EXPECT_TRUE(response.success);
    ASSERT_TRUE(response.response_data.has_value());
    EXPECT_EQ(*response.response_data, id);
}

TEST_F(UsersServiceTests, GetUserIdByUsernameRespondsNotFoundForUnknownUsername) {
    EXPECT_TRUE(service.getUserIdByUsername("nobody").not_found());
}
