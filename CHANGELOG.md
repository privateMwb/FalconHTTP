# Changelog

All notable changes to FalconHTTP are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Nothing yet.

## [1.0.0] - 2026-09-05

The first stable release of FalconHTTP, an embedded C++ HTTP server library
built on this project's own container, caching, concurrency, JSON, and
function-wrapper libraries.

### Added
- Full HTTP/1.1 request/response model: `HttpRequest`/`HttpResponse` with
  case-insensitive header handling (RFC 7230-compliant), query and path
  parameters, and JSON body helpers.
- `HttpParser`/`HttpSerializer`: parses a complete raw request buffer into
  an `HttpRequest`, serializes an `HttpResponse` back to wire-format bytes.
- `Router`: pattern-based routing with `:param` path segments via
  `PathMatcher`, method-aware dispatch that distinguishes `NotFound` (404)
  from `MethodNotAllowed` (405) rather than collapsing both into a generic
  miss.
- Onion-model middleware chain (`Server::use()`), with built-in `Cors`,
  `Logger`, and `Recovery` middleware — the latter converting any uncaught
  exception into a clean 500 instead of taking down the handling thread.
- `StaticFileServer` backed by an LRU `FileCache`, with path-traversal
  protection and MIME-type resolution via `MimeTypes`.
- Thread-pool-backed `Server`, configurable end-to-end via `ServerConfig`
  (port, thread count, and DoS-protection caps).
- Request-size DoS protections: `maxHeaderSize`/`maxBodySize` caps
  returning 431/413 before an oversized request is ever fully buffered.
- Move-only `Socket`/`Connection`/`Listener` — no accidental duplication
  of a native OS handle.
- Built directly on `FunctionPro`, `HashMapPro`, `JsonParser`, `LRUCache`,
  `ThreadPoolPro`, and `VectorPro`.

### Performance
- Header names are normalized to lowercase once, on insert — lookups
  compare directly rather than re-normalizing on every `hasHeader()`/
  `header()` call.
- `HttpSerializer` computes `Content-Length` and `Connection` directly
  from the response at serialization time, rather than buffering first
  and measuring after.
- The LRU-backed `FileCache` avoids re-reading and re-resolving the same
  static file from disk on every repeated request.
- `Server`'s thread pool is constructed once per process and reused
  across every accepted connection, rather than spawning an OS thread
  per request.
- The middleware chain is a straightforward recursive call, not a
  per-request heap-allocated queue — chain overhead is bounded by the
  number of middleware actually registered.
- Benchmarked at 10K / 100K / 1M iterations across parsing, serialization,
  routing, caching, and connection lifecycle. `Router::dispatch()`'s
  linear scan costs ~142x more at 2000 routes than at 10 (the clearest
  case for a trie/radix-based router before large route tables are used);
  `Connection` move construction is effectively free on top of socket
  creation; `Server` construction confirms `ThreadMarket` thread reuse is
  working rather than spawning threads per construction. Full results in
  `benchmarks/results/v1_0_0.md`.

### Testing
- Comprehensive test suite covering unit, integration, lifecycle,
  regression, and concurrency tests: `HttpMethod`, `HttpStatus`,
  `MimeTypes`, `UrlDecoder`, `PathMatcher`, `HttpRequest`/`HttpResponse`,
  `HttpSerializer`, `Socket` options, and `Router` registration/dispatch
  in isolation; the full parse → dispatch → serialize pipeline;
  middleware chain composition and short-circuiting; CORS preflight
  handling; static file serving round trips; route parameter extraction
  through the real parser; move-only RAII and `Server` start/run/stop
  transitions; and thread-safe `Logger`/`Recovery`/`FileCache`/`Router`
  behavior under the shared thread pool.
- Regression coverage pinning every bug fixed during development: request
  body truncation from a header-offset miscalculation, a socket-send
  error silently reported as success, header lookup case-insensitivity,
  oversized-header/body rejection, `405 Method Not Allowed`
  reachability, `Connection: close` on every response, and duplicate
  `Content-Length`/`Connection` header emission.
- 92.8% line coverage (643/693 lines) and 96.6% function coverage
  (114/118 functions), excluding test infrastructure.

### CI
- Conan and vcpkg packaging, each verified with a real consumer smoke
  test against the built package.

[Unreleased]: https://github.com/privateMwb/FalconHTTP/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/privateMwb/FalconHTTP/releases/tag/v1.0.0
