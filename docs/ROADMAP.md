# Roadmap

Everything here was left out of the first release on purpose. It covers the whole system, the server and the
[client](https://github.com/Strangessssss/MessengerClient).

## Next

1. **Saved data.** Add a real database behind the `Database` interface, starting with SQLite or PostgreSQL. This is the
   largest gap for real use. It also allows more than one handler thread, through transactions.
2. **HTTPS.** Passwords and tokens currently travel unencrypted. Add TLS on the server or put it behind a reverse proxy.
3. **Blocking.** Routes to block and unblock, refusing messages from blocked users, and a client page.
4. **Delete a chat for one user.** Deleting should hide the chat only for the user who deletes it, which is not how it works now.
5. **Token expiry in the client**, returning to the login page by itself. For now the user sees an error and presses Ctrl+O.
6. **Message status** in the client's chats list. The server already returns it in `latestMessage`.
7. **Handling an unexpected success when the client creates a chat**, which 6 tests in `ApiServiceCreateChatBadBodyTests` check.
8. **Tab that reaches every field** in the client, matching the `Tab: switch field` hint on the screen.
9. **Password confirmation** on registration in the client.

## Later

- Search with typo tolerance, using PostgreSQL trigrams.
- Editing messages, with an Edited flag.
- Archive, clear and export for chats.
- Settings: language and account deletion.
- Pushing updates over WebSockets instead of polling.
- Resending unsent messages automatically, with client-created IDs so a retry can't create a duplicate.
- Group chats. Storage and `ChatsService` already allow more than 2 users in a chat, but `POST /chats` and the client only
  create chats between 2.

## Known limitations

The client's own are in [its README](https://github.com/Strangessssss/MessengerClient#known-limitations).

- **All data is lost on restart**, and users must log in again.
- **Plain HTTP only**, so anyone on the network path can read messages, passwords and tokens.
- **The server listens on all network interfaces**, with no rate limiting on login attempts.
- **Requests run one at a time.** A password hash blocks all other requests while it runs.
- **Tokens can't be revoked**, and stay valid for 24 hours or until the server restarts. Signing out only makes the client
  forget its token.
- **Deleting a chat deletes it for both users.**
- **Chats list has no fixed order**, as it comes from a hash map. It should be sorted by latest message.
- **Messages are read in full on each poll**, even when nothing changed. A route that returns only newer messages would fix this.
- **An open chat counts as read**, even if the user isn't looking at the window.
- **Messages and usernames have no maximum length.** Usernames are case-sensitive and spaces aren't trimmed, and a message
  of only spaces can be sent.
- **Emoji sent as JSON escapes are stored incorrectly.** Crow decodes surrogate pairs like `\ud83d\ude00` into invalid
  UTF-8. Raw UTF-8 works, and it's what the client sends, so this only affects other API users.

## Hardening and refactoring

- Running the module tests in `tests/module/` from CTest, so one command runs all the server's tests.
- Continuous integration that builds and runs the tests on every push.
- Merge the server and client into one repository, so API changes and both sides of them are reviewed together.
- Load tests, to replace "small number of users" with a real number.
- Sanitizer builds (ASan, UBSan, TSan). TSan would be especially useful on the client's poller.
- Use one naming style in the repositories.
- Share the API types between the server and client, so they can't drift apart.
- An end-to-end test running the real client against the real server.
- Running the client's module tests on Linux, as they've only been run on Windows, and adding them to CTest.
- Tests of the screen logic in the client's `UiService` without a terminal, by moving its state out of the rendering code.
