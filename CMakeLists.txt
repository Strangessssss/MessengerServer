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
        services/ChatsService/DTOs/responses/ChatListResponse.h
        services/ChatsService/DTOs/responses/ChatResponse.h
        services/ChatsService/DTOs/responses/ChatSummaryListResponse.h
        services/ChatsService/DTOs/responses/ChatViewResponse.h
        repositories/MessagesRepository/MessagesRepository.cpp
        repositories/MessagesRepository/MessagesRepository.h
        services/MessagesService/DTOs/responses/MessageResponse.h
        services/MessagesService/DTOs/responses/MessageListResponse.h
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

target_include_directories(MessengerServer PRIVATE ${crossguid_SOURCE_DIR})
target_link_libraries(MessengerServer PRIVATE xg OpenSSL::Crypto Crow::Crow)
if(WIN32)
    # Asio's Windows sockets need these; Crow does not link them under MinGW.
    target_link_libraries(MessengerServer PRIVATE mswsock ws2_32)
endif()
