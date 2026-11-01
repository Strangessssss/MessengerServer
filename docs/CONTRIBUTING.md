# Contributing

## Setup

Follow [Build in the README](README.md#build). The first configure downloads the libraries.

## Structure

Paths on this page start from the repository root. A request passes through these layers, and each one only knows the one
below it:

```
main.cpp              Creates everything and connects it, reads JWT_SECRET and PORT
api/controllers/      Routes: reads the request, calls a service, picks the HTTP status
api/http/             AuthGuard (token check), JsonBody (safe body reading), HttpResponses
api/json/             Converts entities to JSON
services/             The rules, like who can read a chat and what a valid password is
repositories/         Storage operations that services need, and GUID creation
database/             Database (the interface) and MockDatabase (in memory)
entities/, enums/     Plain data: User, Chat, Message, MessageStatus
common/               Response<T>, PasswordHasher, Encoding
```

Each class or entity has its own folder with the same name, like `api/http/JsonBody/JsonBody.cpp` or
`entities/User/User.h`. A service keeps its response types next to it, in `services/<Name>/DTOs/responses/`. `common/` is
grouped by topic instead: `encoding/`, `response/` and `security/`.

## Design notes

- **`Response<T>` has three states.** Success with data, not found (`not_found()`, success without data) and failure
  (`success == false`). Controllers turn these into 2xx, 404 and 400, or 401 for a failed login. Use
  `Response(std::nullopt)` for "doesn't exist" and `Response(std::nullopt, false)` for "rejected", as `not_found()` is
  false for a rejection.
- **Strangers get a 404, not a 403.** A chat the user isn't in appears not to exist, so its ID can't be confirmed.
- **`Database` is an interface** so storage can be replaced by writing one new class and changing one line in `main.cpp`.
  This is the planned path to real storage. Services and repositories must never use `MockDatabase` directly.
- **Handlers run one at a time.** `main.cpp` sets 2 threads, one accepting connections and one running handlers.
  `MockDatabase` has no locks and the services' check-then-write steps rely on this. Raising the thread count needs
  locks or real database transactions first.
- **`MessengerCore` is everything except `main.cpp`**, so tests link exactly the code the server runs.
- **`MessageStatus` has more values than the API uses.** The server only ever sets `Sent` and `Read`. `Pending` and `Error`
  are unused.

## Tests

Unit tests use GoogleTest and live in `tests/`, in folders that follow the source layout. Build and run them with:

```
cmake --build <build dir> --target MessengerServerTests
ctest --test-dir <build dir> --output-on-failure
```

Route tests use `tests/support/ControllerFixture.h`, which connects everything as `main.cpp` does and sends requests to
Crow in-process, so no socket is opened.

Module tests use pytest and live in `tests/module/`. They start the real server binary on a free port and call it over HTTP,
with a new server for each test. They cover registration, login, token checks, tokens across a restart and the main messaging workflow. They look for the binary in `build/` and `cmake-build-*/`, or use
`MESSENGER_SERVER_BIN`. A test marked `xfail` describes a known limitation from the [roadmap](ROADMAP.md#known-limitations),
and fails the run once that limitation is fixed, so remove the mark then. Build the server first, and then:

```bash
python3 -m venv .venv
.venv/bin/pip install -r tests/module/requirements.txt
.venv/bin/python -m pytest tests/module -c tests/module/pytest.ini
```

On Windows, use `.venv\Scripts\` instead of `.venv/bin/`, with MinGW's `bin` folder on `PATH` if the server needs its DLLs.

## Conventions

- C++20, with 4 spaces of indentation. Types are `PascalCase`, functions and variables are `camelCase`. Older repository
  methods use `snake_case`, so new ones should follow the method names around them until these are unified.
- Comments explain why, or a contract that a signature can't show. Headers note how each failure is reported.
- Add new source files to `MessengerCore` in `CMakeLists.txt`, and new test files to `MessengerServerTests`.
- Never hardcode secrets. Configuration comes from environment variables.

## Pull requests

1. Branch from `main`, and keep one change per pull request.
2. Build on Linux, run the tests and check the change by hand.
3. Update the docs in `docs/` when behavior changes: the API table in the README, `SCOPE.md` and `ROADMAP.md`.
4. API changes affect the [client](https://github.com/Strangessssss/MessengerClient), so link its matching pull request.
