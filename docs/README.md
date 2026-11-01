# Messenger Server

The server of a small CLI messenger. It handles registration, login, chats between two users and messages, over a
JSON HTTP API. Users talk to it through the terminal [client](https://github.com/Strangessssss/MessengerClient).

All data is kept in memory, so **everything is lost when the server restarts**, and it uses plain HTTP, so it's only
suitable for local use or a trusted network. See [limitations](ROADMAP.md#known-limitations).

| Document | Contents |
|---|---|
| [SCOPE.md](SCOPE.md) | Scope, functional and non-functional requirements |
| [CONTRIBUTING.md](CONTRIBUTING.md) | Code structure, development setup, tests and conventions |
| [ROADMAP.md](ROADMAP.md) | What is left out of this release and known limitations |
| [ESSAY.md](ESSAY.md) | The project, its organization and technical decisions |

## Build

The target platform is Linux. You need a C++20 compiler (GCC 13 or newer), CMake 3.24 or newer, Git, OpenSSL and libuuid.
On Ubuntu 24.04:

```bash
sudo apt install build-essential cmake ninja-build git libssl-dev uuid-dev
```

Other libraries are downloaded by CMake on the first build, so that build needs internet access.

```bash
git clone https://github.com/Strangessssss/MessengerServer.git
cd MessengerServer
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

CMake also supports Windows with MinGW, such as CLion's bundled toolchain, where it downloads OpenSSL itself. On macOS it
finds Homebrew's `openssl@3`.

This also builds the unit tests. Run them with:

```bash
ctest --test-dir build --output-on-failure
```

## Run

```bash
export JWT_SECRET="$(openssl rand -base64 32)"
./build/MessengerServer
```

| Variable | Required | Meaning |
|---|---|---|
| `JWT_SECRET` | Yes | Signs the login tokens, together with a value unique to each run of the server, so a restart signs everybody out. It must have at least 32 characters, or the server won't start. |
| `PORT` | No | Port to listen on, 8080 by default. It must be a number from 1 to 65535, or the server won't start. |

Then start the [client](https://github.com/Strangessssss/MessengerClient) in another terminal, or use the API directly.

## API

Bodies and responses are JSON, except the empty 204, and errors look like `{"error": "User not found"}`. All routes except the `/auth` ones
need the header `Authorization: Bearer <token>`, and answer 401 without a valid token.

| Method and path | Body | Success | Errors |
|---|---|---|---|
| `POST /auth/register` | `{"username", "password"}` | 201 `{"userId", "token"}` | 400 |
| `POST /auth/login` | `{"username", "password"}` | 200 `{"userId", "token"}` | 400, 401 |
| `GET /chats` | | 200 list of `{"chatId", "partnerUsername", "unreadCount", "latestMessage"}` | |
| `POST /chats` | `{"username"}` | 200 `{"id", "userIds", "createdAt"}`, the new or existing chat | 400, 404 |
| `GET /chats/<id>` | | 200 `{"chatId", "partnerUsername", "messages"}`, marks incoming messages as read | 404 |
| `DELETE /chats/<id>` | | 204, deletes the chat and its messages for **both** users | 404 |
| `POST /chats/<id>/messages` | `{"text"}` | 201 message | 400, 404 |

In `GET /chats`, `latestMessage` is `null` for an empty chat, and otherwise `{"text", "createdAt", "status"}`.

A message looks like this, and its `status` is `sent` or `read`:

```json
{"id": "<guid>", "seq": 1, "chatId": "<guid>", "senderId": 1, "text": "hi", "status": "sent", "createdAt": 1793466103164}
```

`createdAt` is in milliseconds since the epoch.

### Example

```bash
curl -X POST localhost:8080/auth/register -d '{"username":"alice","password":"password123"}'
curl -X POST localhost:8080/auth/register -d '{"username":"bob","password":"password123"}'
TOKEN=<token from the alice response>
curl -X POST localhost:8080/chats -H "Authorization: Bearer $TOKEN" -d '{"username":"bob"}'
```
