# Test Suite

This document describes the test categories under `suite/` — what each
one verifies, and the individual test files it contains.

Unlike the benchmark suite, tests validate the library's own
correctness directly — there is no reference implementation to compare
against, so results are simply pass or fail.

Every test suite registers itself automatically via
`REGISTER_TEST_SUITE()` at startup, and is assigned a sequential id
within its category (e.g. `U1`, `U2` for Unit; `L1`, `L2` for
Lifecycle) — there's no suite list to maintain by hand. This applies
uniformly across every category below.

---

## Concurrency

Verifies thread-safety — Server dispatches requests onto a thread
pool, so multiple requests can be in flight on different threads at
once. Covers shared state accessed from those threads: the mutex-
guarded log writes, and FileCache.

### Tests

- `logger_thread_safety.cpp` — Concurrent requests logging simultaneously don't interleave/garble stdout output
- `recovery_thread_safety.cpp` — Concurrent exceptions caught by Recovery don't interleave stderr output
- `filecache_concurrent_access.cpp` — Concurrent get()/put() from multiple threads stay correct
- `pool_concurrent_requests.cpp` — Many simultaneous connections handled correctly by the thread pool, no cross-request state leakage

---

## Integration

Verifies multiple components working together end-to-end — for
example, a full parse-route-serialize round trip — rather than a
single function in isolation.

### Tests

- `parse_dispatch_respond.cpp` — Full raw-bytes-in to serialized-response-out round trip through HttpParser → Router → HttpSerializer
- `middleware_chain_order.cpp` — Registered middleware runs in onion-model order around the route handler
- `cors_preflight_flow.cpp` — OPTIONS request short-circuits the chain and returns 204 without reaching the handler
- `static_file_roundtrip.cpp` — Static file request resolves, caches, and serves with the correct Content-Type end-to-end
- `route_param_extraction.cpp` — Path pattern match populates the path params the handler actually receives
- `empty_request.cpp` — A connection that sends no data at all closes cleanly without hanging the server
- `header_disconnect.cpp` — A client disconnecting mid-header-block closes cleanly without hanging the server
- `body_disconnect.cpp` — A client disconnecting mid-body closes cleanly without hanging the server
- `parse_exception.cpp` — A request HttpParser::parse() can't parse is caught and reported as 500, not a crash

---

## Lifecycle

Verifies object lifetime operations — construction, destruction, and
moving — across the RAII wrappers around native OS resources (socket
handles) and the Server's start/run/stop state machine.

### Tests

- `socket_raii.cpp` — Socket construction, move, close, and destructor release the handle correctly
- `connection_raii.cpp` — Connection construction, move, and close behave correctly across the underlying Socket
- `listener_bind_close.cpp` — Listener start/stop/isListening across repeated bind cycles
- `server_start_stop.cpp` — Server start/run/stop and isRunning() transitions
- `config_constructor.cpp` — Server(Router&, const ServerConfig&) constructs correctly, and no-arg start() reuses the port it set

---

## Regression

Verifies that a specific, previously fixed bug stays fixed. One test
per resolved issue, added at the time the fix lands.

### Tests

- `body_offset_fix.cpp` — bodyStart must be headerEnd + 4, not headerEnd overwritten to 4 (truncated-body bug)
- `send_error_reported.cpp` — sendAll() must not report a socket error as success (unsigned-underflow bug)
- `header_case_lookup.cpp` — "Content-Type" and "content-type" must resolve to the same header
- `oversized_body_rejected.cpp` — Content-Length beyond maxBodySize is rejected with 413, not read into memory
- `oversized_header_rejected.cpp` — Header block beyond maxHeaderSize is rejected with 431 before the body read begins
- `bad_content_length.cpp` — A Content-Length std::from_chars can't fully parse is rejected with 400, not silently treated as 0
- `method_not_allowed.cpp` — Path matches a route but wrong method returns 405, not 404
- `connection_close_sent.cpp` — Every response includes Connection: close, since keep-alive isn't implemented
- `duplicate_header_fix.cpp` — User-set Content-Length/Connection headers don't get emitted twice

---

## Unit

Verifies individual functions or methods in isolation — the smallest
testable unit of behavior, independent of the categories above.

### Tests

- `url_decode.cpp` — UrlDecoder::decode() percent-decoding and +-as-space
- `path_match.cpp` — PathMatcher::match() segment matching and :param extraction
- `http_method.cpp` — methodFromString()/methodToString() conversions
- `http_status.cpp` — statusReasonPhrase() lookup across all codes
- `mime_lookup.cpp` — mimeTypeFromExtension() known and unknown extensions
- `request_headers.cpp` — HttpRequest header set/get, case-insensitive lookup
- `response_headers.cpp` — HttpResponse header set/get, case-insensitive lookup
- `request_params.cpp` — Query and path parameter set/get
- `response_json.cpp` — setJson() body and Content-Type behavior
- `serializer_output.cpp` — HttpSerializer::serialize() status line, headers, and body assembly
- `socket_options.cpp` — setReuseAddr()/setNonBlocking()/setNoDelay()
- `router_registration.cpp` — get()/post()/put()/del() registration and dispatch matching
