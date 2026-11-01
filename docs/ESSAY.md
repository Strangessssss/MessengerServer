# Essay

## The project

I built a messenger that runs in the terminal. Two users register, start a chat by username and exchange messages through
a central server, seeing whether their messages were read. It has two C++ programs: a server with a JSON HTTP API, and a
client with a full-screen terminal interface.

I chose this project in order to cover as many principles as possible. Even a small messenger needs API design, layered architecture, authentication, access control, concurrency and error handling, so one conversation between two people lets me practise all of them in a single working system.

My first scope was much larger. It included PostgreSQL with typo-tolerant search, blocking, archiving, exporting, message
editing, several languages and automatic resending. I cut all of it. The assignment asks for cutting features rather than
corners, and a messenger's core idea can be shown with one conversation between two people. Those features are now in the
roadmap, along with the reasons they matter. The most significant cut is saved data: the database is a mock in memory, so
a restart loses everything. That would be unacceptable in a real product, and I state it openly rather than hide it.

## The organization

The server has layers: controllers handle HTTP, services hold the rules, repositories offer storage operations, and a
`Database` interface sits at the bottom. Each layer only knows the one below it. This means rules can be tested without
HTTP, and real storage can replace the mock by adding one class and changing one line in `main.cpp`. A single `Response<T>`
type distinguishes success, not found and rejection, which lets controllers pick status codes consistently.

The client has two parts: `ApiService` handles HTTP and `UiService` handles the screen. A background thread polls the
server, and a mutex protects the one value it shares with the UI thread, the ID of the open chat. The thread copies that
value and releases the lock before any network request, so a slow server can't freeze typing.

Automated tests came after most of the code. On the server, GoogleTest covers the rules, storage and routes, including
cases that are hard to reach from outside, like expired tokens and password salting. Route tests send requests to Crow
in-process, without a socket. On the client, tests cover `ApiService`, against a local fake HTTP server, and the JSON
mapping. Python module tests run each real program alone: the server over HTTP, and the client in a pseudo-terminal against
a fake server. The largest remaining gap is that nothing runs the real programs together, which the roadmap plans. The
client's tests also exposed a bug, since fixed: a wrong-shaped answer to creating a chat wasn't handled.

One bug showed the cost of testing late. Creating a chat always failed with "Username missing", though the client seemed to
send the username. The cause was a pair of extra braces in the C++ JSON initializer, which turned `"bob"` into `["bob"]`. It
compiled without warnings, and each side looked correct by itself. The client's tests now check the actual request.

Documentation is split by reader: the READMEs for users, `CONTRIBUTING.md` for contributors with the design rules that
aren't visible from the code, the scope document for requirements, and the roadmap for what isn't done.

## Technological decisions

**C++20** gives `std::jthread` and stop tokens, which make the poller's shutdown simple and safe, as well as ranges.
**CMake with FetchContent** pins dependency versions and downloads them on build, so only a few system packages are needed.
The trade-off is a slow first build that needs internet access.

**Crow** provides the HTTP server with little code and has simple routing, though it's less widely used than some
alternatives. **OpenSSL** provides PBKDF2 and HMAC, as cryptography shouldn't be written by hand. Passwords use salted
PBKDF2 with 600,000 iterations, which is deliberately slow. Because requests run one at a time, a login blocks other
requests while hashing. I accepted this for a small system, and it's in the known limitations.

Tokens are **signed JWTs**, so the server keeps no session list. The cost is that a token can't be revoked before it expires.

**Polling every 2 seconds** rather than WebSockets is the decision I'd most expect to be questioned. It wastes requests and
adds up to 2 seconds of delay. In exchange, the protocol stays plain HTTP, there's no connection state, and when the server
returns, the client simply recovers. For two people chatting, this delay is acceptable.

**FTXUI** provides the terminal interface, **cpp-httplib** the client's HTTP and **nlohmann/json** the client's JSON.
