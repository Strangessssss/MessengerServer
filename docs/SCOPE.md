# Scope and requirements

A CLI messenger: two registered users chat in a terminal through a central server. This document describes what the
system does today, across this server and its [client](https://github.com/Strangessssss/MessengerClient).

The server is a JSON HTTP API, described in the [README](README.md#api). The client is a full-screen terminal program that
connects to `http://127.0.0.1:8080`, or to the address in `MESSENGER_SERVER_URL`.

This release is deliberately small. It does one thing, a conversation between two registered users. Everything else is in
the [roadmap](ROADMAP.md).

## In scope

- Registering, logging in and signing out.
- Starting a chat with another user by their exact username.
- Listing chats, with unread counts and the latest message.
- Closing a chat, and deleting it for both users.
- Sending text messages, and seeing new ones without any action from the user.
- Seeing whether a sent message was read.
- Automated tests of the server, and of the client's HTTP and JSON code, and module tests that run each real program alone.

## Not in scope

These were left out on purpose, as they are not needed to prove the core idea:

- Saved data. The database is a mock that lives in memory.
- Encrypted connections. The server only speaks plain HTTP.
- Search, blocking, settings and languages.
- Editing messages, archiving, clearing and exporting chats, and deleting accounts.
- Group chats. Storage can hold more than 2 users in a chat, but the API and the client only create chats between 2.
- Resending messages automatically, and saving messages or sign in on the client.
- Tests that run the real programs together.

## Functional requirements

### Accounts

* **FR1 - Registration**: Registration requires a username and password. The username can't be empty and must be unique,
  and the password must have at least 8 characters. After registration, the user is signed in automatically.
* **FR2 - Login**: A user logs in with username and password. A wrong password and an unknown username give the same
  error. The client always starts on the login page, as sign in is not saved between runs.
* **FR3 - Sign out**: A user can sign out, which returns them to the login page and removes their data from the screen.
  The token is forgotten by the client, but stays valid on the server until it expires.

### Chats

* **FR4 - Creating**: A user creates a chat by entering the exact username of another user, and the chat opens. A chat is
  only between 2 users. A user can't create a chat with themselves, and an unknown username gives an error. If a chat with
  that user already exists, the existing chat is opened.
* **FR5 - Chats list**: Each entry shows the partner's username, the count of unread messages and the latest message, or
  "(no messages)" if there are none. The count is hidden when nothing is unread, and for the opened chat.
* **FR6 - Closing**: A user can close the opened chat, and it stays in the chats list.
* **FR7 - Deleting**: A user can delete the opened chat after confirming, which deletes the chat and all its messages for
  both users. If the other user has it open, it closes on their next update.
* **FR8 - Privacy**: Only the 2 users of a chat can read it, send to it or delete it. For anybody else the chat appears
  not to exist.

### Messages

* **FR9 - Chat page**: Shows sent and received messages in order, each with the sender and the time, as hours and minutes.
  The newest messages stay in view.
* **FR10 - Sending**: Messages are single-line text, and empty messages are not sent. Whitespace counts as text.
* **FR11 - Message status**: A user's own messages show their status, sent or read. Opening a chat marks all its incoming
  messages as read, and while it is open, new incoming messages are also marked as read.

### Client behavior

* **FR12 - Updates**: After sign in, the client asks the server for new data every 2 seconds, and updates the chats list
  and the opened chat.
* **FR13 - Navigation**: Chats can be chosen with the mouse and arrows. Choosing a chat opens it immediately, and Enter
  or the right arrow moves to the message field. The down arrow past the last chat, or the mouse, reaches the New chat
  field. Tab depends on the current field: with 2 or more chats it cycles through them in the chats list, with 1 chat it
  switches between the list and New chat, and from the message field it returns to the left side. Ctrl+W closes the chat,
  Ctrl+D twice deletes it, Ctrl+O signs out and Esc exits the program. Tab and these hotkeys are listed on the chats page.
* **FR14 - Offline behavior**: When the server is not reachable, the user sees an error on the screen. A message is not
  sent again automatically, but its text stays in the message field so the user can resend it. The client keeps asking
  the server every 2 seconds.
* **FR15 - Rejected token**: When the server rejects the token, for example because it expired, the chats page shows the
  error. The user signs out with Ctrl+O and logs in again, as the client doesn't return to the login page by itself.

## Non-functional requirements

| ID | Requirement | How it is checked |
|---|---|---|
| NFR1 Portability | Builds and runs on Linux, from a clean Ubuntu 24.04 with the packages listed in the README. CMake also supports Windows with MinGW. | Build both repos in a clean `ubuntu:24.04` container |
| NFR2 Security of passwords | Passwords are stored only as salted PBKDF2-HMAC-SHA256 hashes with 600,000 iterations. | `AuthServiceTests`, and code review |
| NFR3 Security of sessions | Every chat and message request needs a valid signed token, which expires after 24 hours. The server refuses to start without a secret of at least 32 characters. | `TokenServiceTests` and the controller tests, and by hand for the startup check |
| NFR4 Trust | The server checks each rule itself, as it never trusts the client. Bad input gets a 4xx error, not a crash. | The controller tests and the module tests |
| NFR5 Reliability of the client | The client keeps running when the server is unreachable or answers with something unexpected, and shows an error. **Not fully met:** a wrong-shaped answer to creating a chat isn't handled, see the [known limitations](ROADMAP.md#known-limitations). | `ApiServiceBehaviorTests`, the client's module tests, and by hand, stopping the server while the client runs |
| NFR6 Latency | A new message appears within about 2 seconds, plus the request time. | By design, as the client polls every 2 seconds |
| NFR7 Load | Meant for a small number of users. Requests are handled one at a time, and no load target is promised in this release. | Not tested, see the roadmap |
| NFR8 Durability | None. All data is lost when the server restarts. This is a stated limit of the release, not a goal. | Not applicable |
| NFR9 Tests | The server's rules, storage and routes have automated tests, and module tests run its real binary over HTTP. The client's HTTP and JSON code also have tests, and module tests run its real binary in a pseudo-terminal against a fake server. Screens not yet covered are checked by hand. | `ctest` and `pytest tests/module` in each repo |

## What "done" means

This release is done when all the functional requirements work from the client on Linux, the tests of both repos pass, and
someone new can get it running from the READMEs alone.
