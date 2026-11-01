# Messenger

A small CLI messenger for Linux, made of two programs in two repositories:

- **Messenger Server** (this repository): registration, login, chats and messages over a JSON HTTP API.
- **[Messenger Client](https://github.com/Strangessssss/MessengerClient)**: the terminal program that users run.

## Start here

| Document | Contents |
|---|---|
| [Server README](docs/README.md) | Build, run and API of the server |
| [Client README](https://github.com/Strangessssss/MessengerClient#readme) | Build, run and use of the client |
| [Scope](docs/SCOPE.md) | Scope, functional and non-functional requirements |
| [Contributing to the server](docs/CONTRIBUTING.md) | Code structure, setup, tests and conventions |
| [Contributing to the client](https://github.com/Strangessssss/MessengerClient/blob/main/CONTRIBUTING.md) | Code structure, setup and tests |
| [Roadmap](docs/ROADMAP.md) | What is left out of this release and known limitations |
| [Essay](docs/ESSAY.md) | The project, its organization and technical decisions |

## Tests

Each repository has C++ unit tests, run with CTest, and Python tests that run its real binary. The contributing guides above
explain how to run them.
