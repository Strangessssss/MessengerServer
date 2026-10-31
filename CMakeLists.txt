cmake_minimum_required(VERSION 4.3)
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
find_package(OpenSSL REQUIRED)

add_executable(MessengerServer main.cpp
        api/controllers/AuthController.cpp
        api/controllers/AuthController.h
        api/controllers/ChatsController.cpp
        api/controllers/ChatsController.h
        api/controllers/MessagesController.cpp
        api/controllers/MessagesController.h
        api/http/AuthGuard.cpp
        api/http/AuthGuard.h
        api/http/HttpResponses.cpp
        api/http/HttpResponses.h
        api/http/JsonBody.cpp
        api/http/JsonBody.h
        api/json/JsonMapper.cpp
        api/json/JsonMapper.h
        entities/User.h
        entities/Chat.h
        entities/Message.h
        enums/MessageStatus.h
        enums/MessageStatus.h
        database/Database.h
        database/MockDatabase.cpp
        database/MockDatabase.h
        services/ChatsService/ChatsService.cpp
        services/ChatsService/ChatsService.h
        services/MessagesService/MessagesService.cpp
        services/MessagesService/MessagesService.h
        repositories/UsersRepository/UsersRepository.cpp
        repositories/UsersRepository/UsersRepository.h
        repositories/ChatsRepository/ChatsRepository.cpp
        repositories/ChatsRepository/ChatsRepository.h
        repositories/ChatsRepository/DTOs/responses/ChatListResponse.h
        repositories/ChatsRepository/DTOs/responses/ChatResponse.h
        repositories/ChatsRepository/DTOs/responses/ChatSummaryListResponse.h
        repositories/ChatsRepository/DTOs/responses/ChatViewResponse.h
        repositories/MessagesRepository/MessagesRepository.cpp
        repositories/MessagesRepository/MessagesRepository.h
        repositories/MessagesRepository/DTOs/responses/MessageResponse.h
        repositories/MessagesRepository/DTOs/responses/MessageListResponse.h
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
)

target_include_directories(MessengerServer PRIVATE ${crossguid_SOURCE_DIR})
target_link_libraries(MessengerServer PRIVATE xg OpenSSL::Crypto Crow::Crow)
