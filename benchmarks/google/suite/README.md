# Google Benchmark Suite

This document describes the benchmark categories under `suite/` — what each
one measures, and the individual benchmarks it contains. Same categories as
`../custom/suite/`, reimplemented with Google Benchmark instead of the
custom framework.

| Category | Focus |
|---|---|
| [Access](#access) | Read and lookup operations against an already-built route table or cache |
| [Core](#core) | Parsing, serializing, chain execution, and cache writes |
| [Lifecycle](#lifecycle) | Construction, destruction, and moving of RAII wrappers around OS resources |
| [Scaling](#scaling) | Per-operation cost as route count, header count, or chain length grows |
| [Utility](#utility) | Small, frequently-called conversion and lookup functions |
| [Conventions](#conventions) | Naming pattern and macro gotchas specific to Google Benchmark |

Unlike VectorPro's suite, there's no `std` equivalent to benchmark FalconHTTP
against — no standard-library HTTP parser, router, or serializer exists to
pair each benchmark with. Every `BENCHMARK()` call below times FalconHTTP
alone, mirroring the custom suite's `BENCH_SOLO()` convention. If a
comparison target (another library, or a minimal hand-rolled baseline) gets
added later, this doc should be revisited.

Iteration count is handled by Google Benchmark itself — each `BENCHMARK()`
runs until `--benchmark_min_time` is satisfied, the Google Benchmark
equivalent of the custom suite's SMALL/MEDIUM/LARGE tiers, without needing
to register separate sizes by hand. The **Scaling** category below measures
something different: how per-operation cost changes as some structural size
(route count, header count, middleware chain length) grows — each tier is
its own `BENCHMARK()` function with a fixed structural size baked in, rather
than an iteration count.

Every benchmark auto-registers via `BENCHMARK(...)` at startup — no suite
list to maintain by hand. Benchmark names double as the filter you'd pass to
`--benchmark_filter`, e.g. `--benchmark_filter=dispatch` runs everything
with "dispatch" in its name. This applies uniformly across every category
below.

---

## Access

Benchmarks read and lookup operations against an already-built route table
or cache — matching a path, resolving a route, and retrieving a cached
entry.

### Benchmarks

| File | What it covers |
|---|---|
| `path_match.cpp` | `PathMatcher::match()` against a fixed pattern: `bench_path_match` |
| `dispatch.cpp` | `Router::dispatch()`: `bench_dispatch_hit`, `bench_dispatch_miss` |
| `match_stream.cpp` | `Router::matchStream()`: `bench_match_stream_hit`, `bench_match_stream_miss` |
| `cache_hit.cpp` | `FileCache::get()` on an existing entry: `bench_cache_hit` |

---

## Core

Benchmarks the fundamental, most frequently exercised operations — parsing
a request, serializing a response, running it through the middleware
chain, and writing into the cache.

### Benchmarks

| File | What it covers |
|---|---|
| `request_line.cpp` | `HttpParser::parse()`, minimal request line + headers: `bench_parse_request_line` |
| `header.cpp` | `HttpParser::parse()` / `HttpSerializer::serialize()`, many headers: `bench_parse_header_heavy`, `bench_serialize_header_heavy` |
| `body.cpp` | `HttpParser::parse()` / `HttpSerializer::serialize()`, large body: `bench_parse_body_heavy`, `bench_serialize_body_heavy` |
| `cache_put.cpp` | `FileCache::put()` insert: `bench_cache_put` |
| `chain_overhead.cpp` | Full middleware chain execution per request: `bench_chain_overhead` |
| `cors_overhead.cpp` | `Cors` middleware execution alone: `bench_cors_overhead` |
| `sse_send.cpp` | `SseConnection::send()` framing + write, on a continuously-drained live connection: `bench_sse_send` |

---

## Lifecycle

Benchmarks object lifetime operations — construction, destruction, and
moving — across the RAII wrappers around native OS resources.

### Benchmarks

| File | What it covers |
|---|---|
| `socket_construction.cpp` | `Socket::createTcp()` construction and close: `bench_socket_construction` |
| `connection_move.cpp` | `Connection` move construction: `bench_connection_move_construction` |
| `server_construction.cpp` | `Server` construction (thread pool startup cost): `bench_server_construction` |

---

## Scaling

Benchmarks how per-operation cost changes as a structural size grows — a
separate axis from Google Benchmark's own min-time-driven iteration count:
each function below fixes a structural size (route count, header count,
middleware chain length) and reports the resulting per-call cost.

### Benchmarks

| File | What it covers |
|---|---|
| `route_table_growth.cpp` | `dispatch()` cost at 10/100/500/2000 registered routes: `bench_dispatch_10_routes`, `bench_dispatch_100_routes`, `bench_dispatch_500_routes`, `bench_dispatch_2000_routes` |
| `header_count_growth.cpp` | `parse()` cost at 5/25/100/500 headers: `bench_parse_5_headers`, `bench_parse_25_headers`, `bench_parse_100_headers`, `bench_parse_500_headers` |
| `middleware_chain_growth.cpp` | Chain execution cost at 1/5/20/50 middleware: `bench_chain_length_1`, `bench_chain_length_5`, `bench_chain_length_20`, `bench_chain_length_50` |

---

## Utility

Benchmarks small, frequently-called conversion and lookup functions that
don't belong to any of the categories above.

### Benchmarks

| File | What it covers |
|---|---|
| `url_decode.cpp` | `UrlDecoder::decode()`: `bench_url_decode` |
| `mime_lookup.cpp` | `mimeTypeFromExtension()`: `bench_mime_lookup_known`, `bench_mime_lookup_unknown` |
| `method_convert.cpp` | `methodFromString()` / `methodToString()`: `bench_method_from_string_best_case`, `bench_method_from_string_worst_case`, `bench_method_to_string` |

---

## Conventions

- **Solo case only** — every benchmark is a single free function,
  `bench_<name>`, registered with its own `BENCHMARK(...)` call. There is
  no `_FalconHTTP`/`_Std` pairing (unlike VectorPro's suite): no baseline
  exists to pair against.
- **Growth tiers** — Scaling-category files register one `BENCHMARK()`
  per fixed structural size (`bench_dispatch_10_routes`,
  `bench_dispatch_100_routes`, ...) rather than using Google Benchmark's
  `Args()`/`Range()` facility, so each tier's size stays visible directly
  in its name and in `--benchmark_filter` output.
- All benchmarks build into a single binary and run through a shared
  `BENCHMARK_MAIN()` entry point defined once outside these files — no
  per-file `BENCHMARK_MAIN()` and no custom suite registration.
- `benchmark::DoNotOptimize(...)` is used in place of the old
  `(void)result;` discards. For anything larger than a register (strings,
  structs, `DispatchResult`), pass the object itself is fine for read-only
  results, but prefer a pointer (`.data()`, `&x`) when benchmarking a
  buffer that's mutated in place.
- Setup that shouldn't be timed (building a `Router`, opening a socket,
  spinning up a drain thread) happens before the `for (auto _ : state)`
  loop, mirroring the custom suite's convention of keeping fixture
  construction outside the timed lambda.
- Use `--benchmark_filter=<regex>` to run a subset, and
  `--benchmark_out=<file> --benchmark_out_format=json` to capture results
  for comparison across runs (e.g. with `compare.py` from the Google
  Benchmark tooling).
