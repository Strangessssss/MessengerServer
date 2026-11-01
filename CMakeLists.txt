cmake_minimum_required(VERSION 3.24)
project(MessengerServer)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

set(XG_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
        crossguid
        GIT_REPOSITORY https://github.com/graeme-hill/crossguid.git
        GIT_TAG v0.2.2
)
FetchContent_Declare(
        asio
        GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
        GIT_TAG asio-1-36-0
)
FetchContent_Declare(
        crow
        GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
        GIT_TAG v1.3.5
)
FetchContent_MakeAvailable(crossguid asio)

# Crow finds Asio through this variable.
set(ASIO_INCLUDE_DIR ${asio_SOURCE_DIR}/asio/include CACHE PATH "" FORCE)
FetchContent_MakeAvailable(crow)

if(APPLE AND NOT OPENSSL_ROOT_DIR AND EXISTS /opt/homebrew/opt/openssl@3)
    set(OPENSSL_ROOT_DIR /opt/homebrew/opt/openssl@3)
endif()
if(MINGW AND NOT OPENSSL_ROOT_DIR)
    # Prebuilt for MinGW-w64 with MSVCRT, matching CLion's bundled toolchain.
    FetchContent_Declare(
            openssl_mingw
            URL https://repo.msys2.org/mingw/mingw64/mingw-w64-x86_64-openssl-3.6.5-1-any.pkg.tar.zst
            URL_HASH SHA256=389D1AF21E99223D9E2CD32D4C073131B276DA85BE7636BD8D23571F6938864F
    )
    FetchContent_MakeAvailable(openssl_mingw)
    set(OPENSSL_ROOT_DIR ${openssl_mingw_SOURCE_DIR}/mingw64)
    set(OPENSSL_USE_STATIC_LIBS TRUE)
endif()
find_package(OpenSSL REQUIRED)

# Everything except main.cpp.
add_library(MessengerCore STATIC
        api/controllers/AuthController/AuthController.cpp
        api/controllers/AuthController/AuthController.h
        api/controllers/ChatsController/ChatsController.cpp
        api/controllers/ChatsController/ChatsController.h
        api/controllers/MessagesController/MessagesController.cpp
        api/controllers/MessagesController/MessagesController.h
        api/http/AuthGuard/AuthGuard.cpp
        api/http/AuthGuard/AuthGuard.h
        api/http/HttpResponses/HttpResponses.cpp
        api/http/HttpResponses/HttpResponses.h
        api/http/JsonBody/JsonBody.cpp
        api/http/JsonBody/JsonBody.h
        api/json/JsonMapper/JsonMapper.cpp
        api/json/JsonMapper/JsonMapper.h
        entities/User/User.h
        entities/Chat/Chat.h
        entities/Message/Message.h
        enums/MessageStatus/MessageStatus.h
        enums/MessageStatus/MessageStatus.h
        database/Database/Database.h
        database/MockDatabase/MockDatabase.cpp
        database/MockDatabase/MockDatabase.h
        services/ChatsService/ChatsService.cpp
        services/ChatsService/ChatsService.h
        services/MessagesService/MessagesService.cpp
        services/MessagesService/MessagesService.h
        repositories/UsersRepository/UsersRepository.cpp
        repositories/UsersRepository/UsersRepository.h
        repositories/ChatsRepository/ChatsRepository.cpp
        repositories/ChatsRepository/ChatsRepository.h
        services/ChatsService/DTOs/responses/ChatListResponse.h
        services/ChatsService/DTOs/responses/ChatResponse.h
        services/ChatsService/DTOs/responses/ChatSummaryListResponse.h
        services/ChatsService/DTOs/responses/ChatViewResponse.h
        repositories/MessagesRepository/MessagesRepository.cpp
        repositories/MessagesRepository/MessagesRepository.h
        services/MessagesService/DTOs/responses/MessageResponse.h
        common/response/Response.h
        common/security/PasswordHasher.cpp
        common/security/PasswordHasher.h
        common/encoding/Encoding.cpp
        common/encoding/Encoding.h
        services/TokenService/TokenService.cpp
        services/TokenService/TokenService.h
        services/AuthService/AuthService.cpp
        services/AuthService/AuthService.h
        services/AuthService/DTOs/responses/AuthResponse.h
        services/UsersService/UsersService.cpp
        services/UsersService/UsersService.h
        services/UsersService/DTOs/responses/UserIdResponse.h
)

target_include_directories(MessengerCore PUBLIC ${crossguid_SOURCE_DIR})
target_link_libraries(MessengerCore PUBLIC xg OpenSSL::Crypto Crow::Crow)
if(WIN32)
    # Asio's Windows sockets need these; Crow does not link them under MinGW.
    target_link_libraries(MessengerCore PUBLIC mswsock ws2_32)
endif()

add_executable(MessengerServer main.cpp)
target_link_libraries(MessengerServer PRIVATE MessengerCore)

include(CTest)
if(BUILD_TESTING)
    # Keeps GoogleTest on the same runtime as the project under MSVC.
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.17.0
    )
    FetchContent_MakeAvailable(googletest)

    add_executable(MessengerServerTests
            tests/database/MockDatabaseTests.cpp
            tests/repositories/UsersRepositoryTests.cpp
            tests/repositories/ChatsRepositoryTests.cpp
            tests/repositories/MessagesRepositoryTests.cpp
            tests/services/UsersServiceTests.cpp
            tests/services/TokenServiceTests.cpp
            tests/services/AuthServiceTests.cpp
            tests/services/ChatsServiceTests.cpp
            tests/services/MessagesServiceTests.cpp
            tests/controllers/AuthControllerTests.cpp
            tests/controllers/AuthGuardTests.cpp
            tests/controllers/ChatsControllerTests.cpp
            tests/controllers/MessagesControllerTests.cpp
    )
    target_include_directories(MessengerServerTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(MessengerServerTests PRIVATE MessengerCore GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(MessengerServerTests)
endif()
