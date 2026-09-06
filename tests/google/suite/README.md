# Google Test Suite

This document describes the test categories under `suite/` — what each
one verifies, and the individual test files it contains. Same
categories as `../custom/suite/`, reimplemented with Google Test
instead of the custom framework.

| Category | Focus |
|---|---|
| [Concurrency](#concurrency) | Thread-safety of shared state accessed from the request thread pool |
| [Integration](#integration) | Multiple components working together end-to-end |
| [Lifecycle](#lifecycle) | Construction, destruction, and moving of RAII wrappers and the Server state machine |
| [Regression](#regression) | A specific, previously fixed bug stays fixed |
| [Unit](#unit) | Individual functions or methods in isolation |
| [Conventions](#conventions) | Naming pattern and macro gotchas specific to Google Test |

Unlike the benchmark suite, tests validate the library's own
correctness directly — there is no reference implementation to compare
against, so results are simply pass or fail.

Every `TEST(...)` call auto-registers at startup — no suite list, and
no manual sequential id (`U1`, `L1`, ...), to maintain by hand. The
suite name (the macro's first argument) plays that organizing role
instead, and doubles as the filter you'd pass to
`--gtest_filter=Suite.*` to run just one file's cases. This applies
uniformly across every category below.

---

## Concurrency

Verifies thread-safety — Server dispatches requests onto a thread
pool, so multiple requests can be in flight on different threads at
once. Covers shared state accessed from those threads: the mutex-
guarded log writes, and FileCache.

### Tests

| File | What it covers |
|---|---|
| `logger_thread_safety.cpp` | Concurrent requests logging simultaneously don't interleave/garble stdout output: `LoggerThreadSafety` |
| `recovery_thread_safety.cpp` | Concurrent exceptions caught by Recovery don't interleave stderr output: `RecoveryThreadSafety` |
| `filecache_concurrent_access.cpp` | Concurrent get()/put() from multiple threads stay correct: `FileCacheConcurrentAccess` |
| `pool_concurrent_requests.cpp` | Many simultaneous connections handled correctly by the thread pool, no cross-request state leakage: `PoolConcurrentRequests` |

---

## Integration

Verifies multiple components working together end-to-end — for
example, a full parse-route-serialize round trip — rather than a
single function in isolation.

### Tests

| File | What it covers |
|---|---|
| `parse_dispatch_respond.cpp` | Full raw-bytes-in to serialized-response-out round trip through HttpParser → Router → HttpSerializer: `ParseDispatchRespond` |
| `middleware_chain_order.cpp` | Registered middleware runs in onion-model order around the route handler: `MiddlewareChainOrder` |
| `cors_preflight_flow.cpp` | OPTIONS request short-circuits the chain and returns 204 without reaching the handler: `CorsPreflightFlow` |
| `static_file_roundtrip.cpp` | Static file request resolves, caches, and serves with the correct Content-Type end-to-end: `StaticFileRoundtrip` |
| `route_param_extraction.cpp` | Path pattern match populates the path params the handler actually receives: `RouteParamExtraction` |
| `empty_request.cpp` | A connection that sends no data at all closes cleanly without hanging the server: `EmptyRequest` |
| `header_disconnect.cpp` | A client disconnecting mid-header-block closes cleanly without hanging the server: `HeaderDisconnect` |
| `body_disconnect.cpp` | A client disconnecting mid-body closes cleanly without hanging the server: `BodyDisconnect` |
| `parse_exception.cpp` | A request HttpParser::parse() can't parse is caught and reported as 500, not a crash: `ParseException` |

---

## Lifecycle

Verifies object lifetime operations — construction, destruction, and
moving — across the RAII wrappers around native OS resources (socket
handles) and the Server's start/run/stop state machine.

### Tests

| File | What it covers |
|---|---|
| `socket_raii.cpp` | Socket construction, move, close, and destructor release the handle correctly: `SocketRaii` |
| `connection_raii.cpp` | Connection construction, move, and close behave correctly across the underlying Socket: `ConnectionRaii` |
| `listener_bind_close.cpp` | Listener start/stop/isListening across repeated bind cycles: `ListenerBindClose` |
| `server_start_stop.cpp` | Server start/run/stop and isRunning() transitions: `ServerStartStop` |
| `config_constructor.cpp` | Server(Router&, const ServerConfig&) constructs correctly, and no-arg start() reuses the port it set: `ConfigConstructor` |

---

## Regression

Verifies that a specific, previously fixed bug stays fixed. One test
per resolved issue, added at the time the fix lands.

### Tests

| File | What it covers |
|---|---|
| `body_offset_fix.cpp` | bodyStart must be headerEnd + 4, not headerEnd overwritten to 4 (truncated-body bug): `BodyOffsetFix` |
| `send_error_reported.cpp` | sendAll() must not report a socket error as success (unsigned-underflow bug): `SendErrorReported` |
| `header_case_lookup.cpp` | "Content-Type" and "content-type" must resolve to the same header: `HeaderCaseLookup` |
| `oversized_body_rejected.cpp` | Content-Length beyond maxBodySize is rejected with 413, not read into memory: `OversizedBodyRejected` |
| `oversized_header_rejected.cpp` | Header block beyond maxHeaderSize is rejected with 431 before the body read begins: `OversizedHeaderRejected` |
| `bad_content_length.cpp` | A Content-Length std::from_chars can't fully parse is rejected with 400, not silently treated as 0: `BadContentLength` |
| `method_not_allowed.cpp` | Path matches a route but wrong method returns 405, not 404: `MethodNotAllowed` |
| `connection_close_sent.cpp` | Every response includes Connection: close, since keep-alive isn't implemented: `ConnectionCloseSent` |
| `duplicate_header_fix.cpp` | User-set Content-Length/Connection headers don't get emitted twice: `DuplicateHeaderFix` |

---

## Unit

Verifies individual functions or methods in isolation — the smallest
testable unit of behavior, independent of the categories above.

### Tests

| File | What it covers |
|---|---|
| `url_decode.cpp` | UrlDecoder::decode() percent-decoding and +-as-space: `UrlDecode` |
| `path_match.cpp` | PathMatcher::match() segment matching and :param extraction: `PathMatch` |
| `http_method.cpp` | methodFromString()/methodToString() conversions: `HttpMethod` |
| `http_status.cpp` | statusReasonPhrase() lookup across all codes: `HttpStatus` |
| `mime_lookup.cpp` | mimeTypeFromExtension() known and unknown extensions: `MimeLookup` |
| `request_headers.cpp` | HttpRequest header set/get, case-insensitive lookup: `RequestHeaders` |
| `response_headers.cpp` | HttpResponse header set/get, case-insensitive lookup: `ResponseHeaders` |
| `request_params.cpp` | Query and path parameter set/get: `RequestParams` |
| `response_json.cpp` | setJson() body and Content-Type behavior: `ResponseJson` |
| `serializer_output.cpp` | HttpSerializer::serialize() status line, headers, and body assembly: `SerializerOutput` |
| `socket_options.cpp` | setReuseAddr()/setNonBlocking()/setNoDelay(): `SocketOptions` |
| `router_registration.cpp` | get()/post()/put()/del() registration and dispatch matching: `RouterRegistration` |

---

## Conventions

- **Suite name = old file identity** — each file's tests share one
  `TEST(SuiteName, CaseName)` suite name (PascalCase, matching the
  file's subject), replacing the old sequential id (`U1`, `L1`, ...)
  as the way to group and filter a file's cases together.
- **Case name = old function name** — each `static void` test
  function becomes one `TEST(..., CaseName)`, with the case name
  converted from snake_case to PascalCase; the function body is
  otherwise unchanged apart from macro substitution.
- **`CHK(x)` becomes `EXPECT_*`/`ASSERT_*`** — a boolean check
  (`CHK(a == b)`, `CHK(!x)`) becomes `EXPECT_EQ`/`EXPECT_TRUE`/
  `EXPECT_FALSE`/`EXPECT_NE` etc. Use `ASSERT_*` instead of `EXPECT_*`
  specifically where a later check in the same test would be
  meaningless or crash if the assertion failed (e.g. asserting a
  pointer is non-null before dereferencing it, or a `start()` call
  succeeded before using the thing it started).
- No per-file `run_tests()` and no `REGISTER_TEST_SUITE()` — Google
  Test's own registration replaces both; every `TEST()` is discovered
  and run automatically.
- All test files build into a single binary and run through a shared
  `main()` provided by linking `gtest_main` (or a small custom `main()`
  calling `RUN_ALL_TESTS()`) — no per-file `main()`.
- Fixture setup shared across a file's cases (e.g. spinning up the
  same `Server` config in `oversized_body_rejected.cpp`'s two cases)
  stays duplicated per-`TEST()` in this suite rather than promoted to
  a `TEST_F` fixture class, mirroring how the custom suite kept each
  `static void` case self-contained.
- Use `--gtest_filter=<pattern>` to run a subset (e.g.
  `--gtest_filter=SocketRaii.*`), and `--gtest_repeat=N` together with
  `--gtest_shuffle` to shake out order-dependent flakiness in the
  Concurrency category specifically.
