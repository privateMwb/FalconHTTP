# Benchmark Suite

This document describes the benchmark categories under `suite/` — what
each one measures, and the individual benchmarks it contains.

| Category | Focus |
|---|---|
| [Access](#access) | Read and lookup operations against an already-built route table or cache |
| [Core](#core) | Parsing, serializing, chain execution, and cache writes |
| [Lifecycle](#lifecycle) | Construction, destruction, and moving of RAII wrappers around OS resources |
| [Scaling](#scaling) | Per-operation cost as route count, header count, or chain length grows |
| [Utility](#utility) | Small, frequently-called conversion and lookup functions |
| [Conventions](#conventions) | Naming pattern and macro gotchas specific to this suite |

Unlike LRUCache's suite, there's no natural "std" equivalent to benchmark
FalconHTTP against — no standard-library HTTP parser, router, or serializer
exists to pair each benchmark with. Every `BENCH_SOLO()` call below times
FalconHTTP alone. If a comparison target (another library, or a minimal
hand-rolled baseline) gets added later, this doc — and the paired-vs-solo
macro choice per file — should be revisited.

Every `BENCH_SOLO()` call, in every category below, is automatically
repeated at three iteration tiers — SMALL (10K), MEDIUM (100K), and
LARGE (1M) — to smooth out timing noise and show whether cost holds
steady as call volume increases, mirroring LRUCache's convention. The
**Scaling** category below measures something different: how
per-operation cost changes as some structural size (route count,
header count, middleware chain length) grows, independent of
iteration count.

---

## Access

Benchmarks read and lookup operations against an already-built route
table or cache — matching a path, resolving a route, and retrieving a
cached entry.

### Benchmarks

| File | What it covers |
|---|---|
| `path_match.cpp` | `PathMatcher::match()` against a fixed pattern |
| `dispatch.cpp` | `Router::dispatch()` on a matching method + path, and on a path with no matching route |
| `match_stream.cpp` | `Router::matchStream()` on a matching method + path, and on a path with no matching Stream-kind route |
| `cache_hit.cpp` | `FileCache::get()` on an existing entry |

---

## Core

Benchmarks the fundamental, most frequently exercised operations —
parsing a request, serializing a response, running it through the
middleware chain, and writing into the cache.

### Benchmarks

| File | What it covers |
|---|---|
| `request_line.cpp` | `HttpParser::parse()`, minimal request line + headers |
| `header.cpp` | `HttpParser::parse()` and `HttpSerializer::serialize()`, many headers |
| `body.cpp` | `HttpParser::parse()` and `HttpSerializer::serialize()`, large body |
| `cache_put.cpp` | `FileCache::put()` insert |
| `chain_overhead.cpp` | Full middleware chain execution per request |
| `cors_overhead.cpp` | `Cors` middleware execution alone |
| `sse_send.cpp` | `SseConnection::send()` event framing + write, on a continuously-drained live connection |

---

## Lifecycle

Benchmarks object lifetime operations — construction, destruction,
and moving — across the RAII wrappers around native OS resources.

### Benchmarks

| File | What it covers |
|---|---|
| `socket_construction.cpp` | `Socket::createTcp()` construction and close |
| `connection_move.cpp` | `Connection` move-construct and move-assign |
| `server_construction.cpp` | `Server` construction (thread pool startup cost) |

---

## Scaling

Benchmarks how per-operation cost changes as a structural size grows
— a separate axis from the SMALL/MEDIUM/LARGE iteration tiers above:
those repeat the same fixed-size operation more times, while Scaling
grows the structure itself (route table, header count, middleware
chain) and observes the resulting per-call cost. Directly quantifies
`Router::dispatch()`'s linear-scan cost as route count grows.

### Benchmarks

| File | What it covers |
|---|---|
| `route_table_growth.cpp` | `dispatch()` cost as registered route count increases |
| `header_count_growth.cpp` | Parse/serialize cost as header count increases |
| `middleware_chain_growth.cpp` | Chain execution cost as middleware count increases |

---

## Utility

Benchmarks small, frequently-called conversion and lookup functions
that don't belong to any of the categories above.

### Benchmarks

| File | What it covers |
|---|---|
| `url_decode.cpp` | `UrlDecoder::decode()` |
| `mime_lookup.cpp` | `mimeTypeFromExtension()` |
| `method_convert.cpp` | `methodFromString()` / `methodToString()` |