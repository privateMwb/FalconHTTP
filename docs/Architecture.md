# FalconHTTP — Architecture

## Overview

FalconHTTP is a multithreaded HTTP/1.1 server built entirely from scratch in C++23 — raw TCP sockets, a hand-written HTTP parser, a custom router, and a deliberately small middleware system, with no external networking or HTTP libraries. It reuses the author's own libraries from earlier projects (`HashMap`, `VectorPro`, `FunctionPro`, `LRUCache`, `Json`/`Parser`, `ThreadPool`) rather than the C++ standard library's equivalents, continuing the "build your own tools" philosophy carried across this whole portfolio.

The goal was never to compete with production web servers — it was to understand, by building, what actually happens between a browser sending bytes and a server sending bytes back.

## Request Lifecycle

A single incoming request travels through the following stages, in order:

1. **`Listener::accept()`** — blocks on the listening socket until a client connects, returns a raw `Socket`.
2. **`Connection`** wraps that socket, providing reliable `sendAll()` (loops until everything is sent) and `receiveAvailable()` (one chunk at a time).
3. **`Server::handleConnection()`** reads chunks until the header section is complete, scans for `Content-Length`, then keeps reading until the full body has arrived.
4. **`HttpParser::parse()`** turns the complete raw buffer into a structured `HttpRequest` — method, path, version, headers, query params, body.
5. **The middleware chain** runs, in registration order: `Recovery` (outermost, wraps everything in a try/catch) → `Logger` (times and logs the whole downstream call) → `Cors` (adds headers, short-circuits on `OPTIONS` preflight).
6. **`Router::dispatch()`** walks registered routes, matches method + path (via `PathMatcher`, which also extracts `:param` segments), and calls the matching handler — or reports no match, letting `Server` build a `404`.
7. **The route handler** reads the request and fills in an `HttpResponse`.
8. **`HttpSerializer::serialize()`** turns the response back into raw bytes — status line, headers (with `Content-Length` computed automatically), blank line, body.
9. **`Connection::sendAll()`** sends it back, and the connection closes.

Each accepted connection runs this entire sequence on its own `ThreadPool` worker thread, dispatched via `Server::run()`'s `pool_.execute(...)` call — the accept loop itself stays single-threaded (sequential by nature, since it's reading from one socket), while every client's actual request handling happens concurrently.

## The Middleware Model

Middleware uses an "onion" / wrapping model: `void(HttpRequest&, HttpResponse&, const NextHandler& next)`. Each middleware decides whether and when to call `next(request, response)` — which recursively continues the chain. This was a deliberate choice over a simpler "pre/post hook" model, because `Recovery` needs to wrap a `try`/`catch` around *everything* downstream of it, including the route handler itself — something hooks fundamentally can't express, since nothing in a hook model nests the rest of the chain inside one call frame. `Cors` relies on the same property in reverse: it can choose to **not** call `next()` at all for an `OPTIONS` preflight request, short-circuiting the chain entirely.

Registration order matters: `Recovery` should always be registered first (outermost), so it can catch exceptions from every other middleware, not just the final handler.

## Design Decisions and Tradeoffs

- **Single-call body reads, with a `Content-Length`-aware completion loop.** Early on, `Server::handleConnection()` read the request in one `receiveAvailable()` call — fine for small requests, silently truncating for anything larger. This was fixed by scanning for `Content-Length` after the headers arrive and looping until the full body is present. The scan currently lives inline in `Server.cpp` rather than as an exposed `HttpParser` helper — a known, intentional piece of duplication, flagged as a refactor candidate rather than fixed, since `HttpParser`'s public contract was deliberately kept stable.
- **No chunked transfer encoding.** Only `Content-Length`-declared bodies are supported, consistent with the project's stated scope (chunked encoding is listed as a future feature).
- **Static file serving uses `std::filesystem` for path-safety, not hand-rolled string checks.** This was one of the few deliberate departures from "build everything yourself" — path traversal protection is security-critical, and getting it wrong by hand (symlinks, encoded `..`, platform separator differences) carries real risk for low learning payoff compared to, say, building a hash map. `isPathSafe()` resolves the target path via `weakly_canonical()` and compares it component-by-component against the canonical root.
- **`FileCache` trades strict const-correctness for real LRU behavior.** `FileCache::get()` is non-const because it needs to call the cache's real `get()` (which promotes an entry to most-recently-used) rather than a const-safe `peek()` that would never refresh recency — defeating the purpose of an LRU cache. This ripples into `StaticFileServer::serve()` also needing to be non-const.
- **No size cutoff on cached files.** Capacity is bounded by entry *count* (via `LRUCache`), not total bytes. A byte-aware capacity model was considered out of scope for v1.
- **A custom type-erased `Function` (not `std::function`) for `RouteHandler` and `MiddlewareFn`.** This is the project's own small-buffer-optimized callable wrapper, used in place of the standard library equivalent, consistent with the portfolio's broader philosophy.

## Files Deliberately Not Built

A few files from the original project structure were consciously dropped rather than left as unbuilt placeholders, each with real reasoning:

- **`Client.h`** — once `Connection` and `Server` existed, there was no remaining gap for a separate `Client` abstraction to fill. It would only matter if FalconHTTP needed to make *outbound* HTTP requests (e.g. as a reverse proxy), which is outside the project's scope.
- **`QueryParser.h` / `HeaderParser.h`** — their logic already lives correctly inside `HttpParser` (`parsePath`, `parseHeaderLine`). Extracting them into separate files would have been pure relocation with no functional benefit, since nothing else needs that logic independently.
- **`StringUtils.h`** — no real call site ever needed a generic string-utilities file; every small string operation was written inline exactly where needed. Building it speculatively would have meant dead code.

## Bugs Found During Testing

Two real bugs were found and fixed through actual end-to-end testing (manual `curl` requests, later backed by automated tests), worth recording since both were subtle enough to pass code review by inspection alone:

1. **`PathMatcher` segment-parameter detection.** Segments included their leading `/` (e.g. `"/users"`, not `"users"`), but the parameter check tested `segment.front() == ':'` — which can never be true, since the front character is always `/`. This made every `:param` route silently fail to match, always falling through to a literal comparison that never matched either. Fixed by checking `segment[1] == ':'` and adjusting the substring offsets accordingly for both the parameter name and captured value.
2. **`PathMatcher` segment-count mismatch handling.** When either `pattern` or `path` ran out of further `/` segments, the code forced *both* strings to empty immediately — incorrectly treating "one side is exhausted" as "both sides are exhausted." This caused `/users/:id` to incorrectly match `/users` (too few segments) and `/users` to incorrectly match `/users/42` (too many). Fixed by letting each side collapse to empty independently, only when *that side specifically* has no more segments.

Both were caught via automated tests in `tests/test_pathmatcher.cpp`, which directly encode these exact failure cases as permanent regressions.

A third issue, found via manual testing rather than a logic bug, was that `Json::operator[]` does not auto-promote a default-constructed (`Null`-typed) `Json` into an `Object` — calling `body["key"] = value` on a fresh `Json` throws. The fix was constructing the `Json::ObjectType` map directly and building the `Json` from it, rather than relying on `[]`-based incremental construction.

## Platform-Specific Issues (Windows)

Developing and testing on Windows/MinGW surfaced several issues that wouldn't occur on Linux/Mac, none of which affected the underlying design:

- **Winsock requires explicit initialization.** Unlike POSIX sockets, Windows requires `WSAStartup()` before any socket call will reliably work, and `WSACleanup()` at shutdown. This is handled via a private, file-scope static object in `Socket.cpp` whose constructor/destructor run automatically before `main()` and after it returns, guarded by `#ifdef _WIN32` — invisible to every other file in the project.
- **`::close()` vs `::closesocket()`.** POSIX file descriptors and Windows socket handles are closed via different functions; `Socket::close()` is platform-split accordingly.
- **A ~200ms latency mystery that turned out to be neither a bug nor `TCP_NODELAY`.** Initial `curl` timing showed a consistent ~200ms round-trip despite server-side logging showing sub-millisecond handler execution. Detailed `curl` timing breakdown isolated the delay to the TCP *connect* phase specifically. The actual cause: `curl` (and most clients) resolve `localhost` to both `::1` (IPv6) and `127.0.0.1` (IPv4), trying IPv6 first; since `Listener` only binds an IPv4 socket, every connection attempt waited through an IPv6 connection failure/timeout before falling back to IPv4. Testing against `127.0.0.1` directly (or passing `curl -4`) confirmed this completely — the server itself was never slow.

## Known Limitations

- **Graceful shutdown has only been verified on this development platform.** `Server::stop()` closes the listening socket to unblock a thread sitting inside `accept()`; this was tested via a `SIGINT` handler and confirmed to work correctly on Windows/MinGW, but POSIX-documented behavior for this exact scenario can vary across platforms, so it isn't a universal guarantee.
- **IPv4 only.** `Listener` does not bind an IPv6 socket, meaning clients resolving `localhost` to `::1` first will experience a connection-fallback delay before reaching the server (see above).
- **No chunked transfer encoding, compression, or persistent connections** — all explicitly listed as future work in the original project scope.
- **`Server`, `Connection`, and `Socket` have no automated unit tests.** These are fundamentally about real TCP I/O; testing them meaningfully would require either spinning up a real server (an integration test, which manual `curl` verification already covers) or mocking the OS socket layer. This was a conscious scope decision, not an oversight.
- **No benchmarks.** Unlike MiniDatabase, where performance characteristics of custom data structures were a central point of the project, FalconHTTP's value lies in correctly implementing protocol and architecture layers — raw throughput numbers would mostly measure the OS's TCP stack rather than anything this project specifically built.

## Test Coverage

Automated tests (in `tests/`, using a lightweight `assert`-style helper, no external framework) cover:
- `PathMatcher` — literal matching, parameter capture, segment-count mismatches, multiple params.
- `HttpParser` — request line parsing, headers, query strings (including percent-decoding), bodies, malformed input.
- `Router` — dispatch matching, method mismatches, path param wiring, registration-order precedence.
- `UrlDecoder` — percent-decoding, `+`-as-space, malformed sequence leniency.
- `HttpSerializer` — status line format, automatic `Content-Length`, header/body structure.
- `StaticFileServer` — file serving, MIME type resolution, nested paths, path traversal blocking, cache behavior.
- `Middleware` (`Logger`, `Cors`, `Recovery`) — chain continuation, CORS preflight short-circuiting, exception recovery (both standard and non-standard exceptions).