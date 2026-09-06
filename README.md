<p align="center">
  <img src=".github/assets/banner.svg" alt="FalconHTTP" width="100%">
</p>

<p align="center">
  <img src="https://img.shields.io/github/v/release/privateMwb/FalconHTTP?style=for-the-badge&logo=github&color=2563EB&labelColor=03100D" alt="Version">
  <img src="https://img.shields.io/badge/License-MIT-0D9488?style=for-the-badge&labelColor=03100D" alt="License - MIT">
  <img src="https://img.shields.io/badge/C%2B%2B-23-22C55E?style=for-the-badge&labelColor=03100D" alt="C++ - 23">
</p>

<p align="center">
  <img src=".github/assets/divider.svg" alt="" width="100%">
</p>

<p align="center"><sub><b>CI / CD</b></sub></p>
<p align="center">
  <a href="https://github.com/privateMwb/FalconHTTP/actions/workflows/build.yml">
    <img src="https://github.com/privateMwb/FalconHTTP/actions/workflows/build.yml/badge.svg" alt="Build and Test">
  </a>
  <a href="https://github.com/privateMwb/FalconHTTP/actions/workflows/benchmark.yml">
    <img src="https://github.com/privateMwb/FalconHTTP/actions/workflows/benchmark.yml/badge.svg" alt="Benchmarks">
  </a>
  <a href="https://github.com/privateMwb/FalconHTTP/actions/workflows/packaging.yml">
    <img src="https://github.com/privateMwb/FalconHTTP/actions/workflows/packaging.yml/badge.svg" alt="Packaging">
  </a>
  <a href="https://github.com/privateMwb/FalconHTTP/actions/workflows/release.yml">
    <img src="https://github.com/privateMwb/FalconHTTP/actions/workflows/release.yml/badge.svg" alt="Release">
  </a>
</p>

<p align="center"><sub><b>Code Quality &amp; Safety</b></sub></p>
<p align="center">
  <a href="https://github.com/privateMwb/FalconHTTP/actions/workflows/coverage.yml">
    <img src="https://github.com/privateMwb/FalconHTTP/actions/workflows/coverage.yml/badge.svg" alt="Coverage">
  </a>
  <a href="https://github.com/privateMwb/FalconHTTP/actions/workflows/sanitizers.yml">
    <img src="https://github.com/privateMwb/FalconHTTP/actions/workflows/sanitizers.yml/badge.svg" alt="Sanitizers">
  </a>
  <a href="https://github.com/privateMwb/FalconHTTP/actions/workflows/clang-tidy.yml">
    <img src="https://github.com/privateMwb/FalconHTTP/actions/workflows/clang-tidy.yml/badge.svg" alt="Clang Tidy">
  </a>
  <a href="https://github.com/privateMwb/FalconHTTP/actions/workflows/clang-format.yml">
    <img src="https://github.com/privateMwb/FalconHTTP/actions/workflows/clang-format.yml/badge.svg" alt="Clang Format">
  </a>
  <a href="https://github.com/privateMwb/FalconHTTP/actions/workflows/codeql.yml">
    <img src="https://github.com/privateMwb/FalconHTTP/actions/workflows/codeql.yml/badge.svg" alt="CodeQL">
  </a>
  <a href="https://github.com/privateMwb/FalconHTTP/actions/workflows/cflite_pr.yml">
    <img src="https://github.com/privateMwb/FalconHTTP/actions/workflows/cflite_pr.yml/badge.svg" alt="Fuzzing">
  </a>
  <a href="https://www.bestpractices.dev/projects/14466">
    <img src="https://www.bestpractices.dev/projects/14466/badge" alt="OpenSSF Best Practices">
  </a>
</p>

<p align="center"><sub><b>Documentation</b></sub></p>
<p align="center">
  <a href="https://github.com/privateMwb/FalconHTTP/actions/workflows/docs.yml">
    <img src="https://github.com/privateMwb/FalconHTTP/actions/workflows/docs.yml/badge.svg" alt="Documentation">
  </a>
</p>

<p align="center">
  <img src=".github/assets/divider.svg" alt="" width="100%">
</p>

<p align="center"><sub><b>Compiler Support</b></sub></p>
<p align="center">
  <img src="https://img.shields.io/badge/GCC-support-B46F1B?style=flat&logo=gnu" alt="GCC - support">
  <img src="https://img.shields.io/badge/Clang-support-045891?style=flat&logo=llvm" alt="Clang - support">
  <img src="https://img.shields.io/badge/MSVC-support-5C2D91?style=flat" alt="MSVC - support">
  <img src="https://img.shields.io/badge/AppleClang-support-000000?style=flat&logo=apple" alt="AppleClang - support">
</p>

<p align="center">
  <img src=".github/assets/divider.svg" alt="" width="100%">
</p>

<p align="center">FalconHTTP is an embedded C++ HTTP server library — thread-pool-backed request handling, pattern-based routing with path parameters, Server-Sent Events streaming, an onion-model middleware chain, and static file serving backed by an LRU cache, built entirely on this author's own container, caching, concurrency, JSON, and function-wrapper libraries.</p>

<br>

## 📑 Table of Contents

- [Features](#features)
- [Requirements](#requirements)
- [Dependencies](#dependencies)
- [Installation](#installation)
- [Quick Start](#quick-start)
- [Project Structure](#project-structure)
- [Development](#development)
- [Benchmarks](#benchmarks)
- [Fuzzing](#fuzzing)
- [Documentation](#documentation)
- [Contributing](#contributing)
- [Changelog](#changelog)
- [License](#license)

<br>

## <a id="features"></a>✨ Features

- **Case-insensitive header handling** — `HttpRequest`/`HttpResponse` normalize header names to lowercase on both insert and lookup (RFC 7230 §3.2), so `Content-Type` and `content-type` always resolve to the same entry.
- **Method-aware routing** — `Router::dispatch()` distinguishes a path with no matching route (`NotFound`/404) from a path that matches but not for the requested method (`MethodNotAllowed`/405), rather than collapsing both into a generic miss.
- **Server-Sent Events streaming** — `Router::stream()` registers a `StreamHandler` given an `SseConnection` instead of a one-shot `HttpResponse`, able to push framed events over a single held-open connection for as long as the client stays connected; disconnects are detected on the next failed write, not assumed. `ServerConfig::maxStreamingConnections` caps how many can run at once (half the thread pool by default) so a burst of subscribers can't starve every other in-flight request behind the pool's unbounded queue.
- **Onion-model middleware chain** — `Server::use()` registers middleware that wraps the handler in registration order, with `Cors`, `Logger`, and `Recovery` built in; `Recovery` converts any uncaught exception into a clean 500 instead of taking down the handling thread. Headers a middleware sets (e.g. `Cors`) are honored on streaming routes too, forwarded into the SSE preamble.
- **Request-size DoS protection** — `ServerConfig::maxHeaderSize`/`maxBodySize` reject an oversized header block or body with 431/413 before it's ever fully buffered into memory.
- **Static file serving with an LRU cache** — `StaticFileServer` resolves and serves files under a fixed root directory, with path-traversal protection and a `FileCache` avoiding repeated disk reads for hot files.
- **Move-only RAII sockets** — `Socket`/`Connection`/`Listener` can't have their underlying OS handle silently duplicated by an accidental copy.
- **Thread-pool-backed `Server`** — one thread pool per process, reused across every accepted connection rather than spawning an OS thread per request.

<div align="right"><a href="#-table-of-contents"><img src=".github/assets/back-to-top.svg" alt="Back to top" height="28"></a></div>

## <a id="requirements"></a>📋 Requirements

- A C++23-conformant compiler (tested: GCC, Clang, MSVC, AppleClang)
- CMake 3.20+
- Git submodules initialized — unlike this author's other, dependency-free libraries, FalconHTTP is a consumer of 6 of them (see [Dependencies](#dependencies)) and needs their source present to build from source

<div align="right"><a href="#-table-of-contents"><img src=".github/assets/back-to-top.svg" alt="Back to top" height="28"></a></div>

## <a id="dependencies"></a>🔗 Dependencies

FalconHTTP is built entirely on this author's own libraries, vendored as git submodules under `libs/internal/`:

| Library | Provides | Repository |
|---|---|---|
| VectorPro | `Vector<T>`, backing `Router::routes` and `Server`'s registered middleware list | [privateMwb/VectorPro](https://github.com/privateMwb/VectorPro) |
| JsonPro | `Json`, the body format for `HttpRequest::json()` / `HttpResponse::setJson()` | [privateMwb/JsonParser](https://github.com/privateMwb/JsonParser) |
| FunctionPro | `Function<>`, wrapping every `RouteHandler`, `StreamHandler`, `MiddlewareFn`, and `NextHandler` callback | [privateMwb/FunctionPro](https://github.com/privateMwb/FunctionPro) |
| HashMapPro | `HashMap<K,V>`, backing header, query, and path parameter storage on every request and response | [privateMwb/HashMapPro](https://github.com/privateMwb/HashMapPro) |
| CachePro | `LRUCache<K,V>`, backing `FileCache`'s static file content cache | [privateMwb/LRUCache](https://github.com/privateMwb/LRUCache) |
| ThreadPoolPro | `ThreadPool`, driving `Server`'s per-connection thread pool | [privateMwb/ThreadPoolPro](https://github.com/privateMwb/ThreadPoolPro) |

<div align="right"><a href="#-table-of-contents"><img src=".github/assets/back-to-top.svg" alt="Back to top" height="28"></a></div>

## <a id="installation"></a>📦 Installation

**From source:**

```bash
git clone --recurse-submodules https://github.com/privateMwb/FalconHTTP.git
cd FalconHTTP
cmake -B build \
  -DBUILD_TESTS=OFF \
  -DBUILD_BENCHMARKS=OFF \
  -DBUILD_REGRESSION=OFF \
  -DBUILD_EXAMPLES=OFF
cmake --install build
```

Then, in your own `CMakeLists.txt`:

```cmake
find_package(FalconHTTP CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE FalconHTTP::FalconHTTP)
```

> vcpkg and Conan packages are built and verified (recipe in
> `packaging/recipes/falconhttp/`, port in `packaging/vcpkg/ports/falconhttp/`),
> but not yet published to the public registries. This section will be
> updated once they are.

<div align="right"><a href="#-table-of-contents"><img src=".github/assets/back-to-top.svg" alt="Back to top" height="28"></a></div>

## <a id="quick-start"></a>🚀 Quick Start

```cpp
#include <FalconHTTP/Core/Server.h>
#include <FalconHTTP/Routing/Router.h>
#include <FalconHTTP/Middleware/Logger.h>

using namespace FalconHTTP::Core;
using namespace FalconHTTP::Routing;
using namespace FalconHTTP::HTTP;
using namespace FalconHTTP::Middleware;

int main() {
    Router router;

    router.get("/users/:id", [](const HttpRequest& request, HttpResponse& response) {
        response.setStatus(HttpStatus::Ok);
        response.setBody("user id: " + request.pathParam("id"));
    });

    Server server(router, /*threadCount=*/4);
    server.use(Logger{});

    if (!server.start(8080)) {
        return 1;
    }

    server.run(); // blocks until server.stop() is called from another thread
}
```

Streaming Server-Sent Events — the connection stays open and `send()`
can be called repeatedly for as long as the client is connected:

```cpp
#include <FalconHTTP/Streaming/SseConnection.h>

using namespace FalconHTTP::Streaming;

router.stream("/events", [](const HttpRequest&, SseConnection& sse) {
    for (int i = 0; i < 10 && sse.isConnected(); ++i) {
        sse.send("tick", "count:" + std::to_string(i));
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
});
```

Serving static files, with CORS enabled:

```cpp
#include <FalconHTTP/FileServer/StaticFileServer.h>
#include <FalconHTTP/Middleware/Cors.h>

using namespace FalconHTTP::FileServer;

StaticFileServer files("./public", /*cacheCapacity=*/256);

router.get("/index.html", [&files](const HttpRequest& request, HttpResponse& response) {
    if (!files.serve(request.path(), response)) {
        response.setStatus(HttpStatus::NotFound);
    }
});

server.use(Cors{"https://example.com"});
```

FalconHTTP surfaces most contract violations by returning a sentinel value rather than throwing (`Router::dispatch()` returns a `DispatchResult`, `HttpRequest::header()` returns an empty string for a missing key) — the one place it does throw is malformed input to `HttpParser::parse()`, which the `Recovery` middleware converts into a clean 500 rather than crashing the handling thread:

```cpp
server.use(Recovery{}); // must be registered before anything that might throw

router.get("/risky", [](const HttpRequest&, HttpResponse&) {
    throw std::runtime_error("something went wrong");
    // Recovery catches this and responds 500 Internal Server Error
});
```

<div align="right"><a href="#-table-of-contents"><img src=".github/assets/back-to-top.svg" alt="Back to top" height="28"></a></div>

## <a id="project-structure"></a>🗂️ Project Structure

```
FalconHTTP/
├── include/
│   └── FalconHTTP/
│       ├── Core/
│       │   ├── Socket.h
│       │   ├── Listener.h
│       │   ├── Connection.h
│       │   └── Server.h
│       ├── HTTP/
│       │   ├── HttpMethod.h
│       │   ├── HttpStatus.h
│       │   ├── HttpRequest.h
│       │   ├── HttpResponse.h
│       │   ├── HttpParser.h
│       │   ├── HttpSerializer.h
│       │   └── MimeTypes.h
│       ├── Routing/
│       │   ├── Route.h
│       │   ├── RouteHandler.h
│       │   ├── StreamHandler.h
│       │   ├── PathMatcher.h
│       │   └── Router.h
│       ├── Streaming/
│       │   └── SseConnection.h
│       ├── Middleware/
│       │   ├── Middleware.h
│       │   ├── Cors.h
│       │   ├── Logger.h
│       │   └── Recovery.h
│       ├── FileServer/
│       │   ├── FileCache.h
│       │   └── StaticFileServer.h
│       ├── Utility/
│       │   └── UrlDecoder.h
│       ├── Config/
│       │   └── ServerConfig.h
│       └── FalconHTTP.h
│
├── src/
│   └── FalconHTTP/
│       ├── Core/
│       │   ├── Socket.cpp
│       │   ├── Listener.cpp
│       │   ├── Connection.cpp
│       │   └── Server.cpp
│       ├── HTTP/
│       │   ├── HttpMethod.cpp
│       │   ├── HttpStatus.cpp
│       │   ├── HttpRequest.cpp
│       │   ├── HttpResponse.cpp
│       │   ├── HttpParser.cpp
│       │   ├── HttpSerializer.cpp
│       │   └── MimeTypes.cpp
│       ├── Routing/
│       │   ├── Route.cpp
│       │   ├── PathMatcher.cpp
│       │   └── Router.cpp
│       ├── Streaming/
│       │   └── SseConnection.cpp
│       ├── Middleware/
│       │   ├── Cors.cpp
│       │   ├── Logger.cpp
│       │   └── Recovery.cpp
│       ├── FileServer/
│       │   ├── FileCache.cpp
│       │   └── StaticFileServer.cpp
│       └── Utility/
│           └── UrlDecoder.cpp
│
├── libs/
│   └── internal/
│       ├── VectorPro/
│       ├── JsonParser/
│       ├── FunctionPro/
│       ├── HashMapPro/
│       ├── LRUCache/
│       └── ThreadPoolPro/
│
├── tests/
│   ├── custom/
│   ├── google/
│   ├── CMakeLists.txt
│   └── README.md
│
├── benchmarks/
│   ├── custom/
│   ├── google/
│   ├── baselines/
│   ├── results/
│   ├── CMakeLists.txt
│   └── README.md
│
├── examples/
│   ├── support/
│   ├── suite/
│   ├── example_main.cpp
│   └── CMakeLists.txt
│
├── regression/
│   ├── custom/
│   ├── google/
│   ├── results/
│   ├── CMakeLists.txt
│   └── README.md
│
├── fuzz/
│   ├── fuzz_http_parser.cpp
│   └── fuzz_path_matcher.cpp
│
├── .clusterfuzzlite/
│   ├── Dockerfile
│   ├── build.sh
│   └── project.yaml
│
├── packaging/
│   ├── README.md
│   ├── requirements-conan.txt
│   ├── recipes/
│   ├── vcpkg/
│   └── vcpkg-smoke-test/
│
├── scripts/
│   └── update_package_files.py
│
├── .github/
│   ├── releases/
│   └── workflows/
│
├── cmake/
│   └── FalconHTTPConfig.cmake.in
│
├── docs/
│   ├── Doxyfile
│   └── README.md
│
├── .clang-format
├── .clang-tidy
├── .gitignore
├── CMakeLists.txt
├── README.md
├── CONTRIBUTING.md
├── CHANGELOG.md
├── SECURITY.md
├── FUZZING.md
└── LICENSE
```

<div align="right"><a href="#-table-of-contents"><img src=".github/assets/back-to-top.svg" alt="Back to top" height="28"></a></div>

## <a id="development"></a>🛠️ Development

The from-source install above builds the library only. To work on
FalconHTTP itself — running tests, benchmarks, or the regression tool —
build with everything enabled (the default):

```bash
cmake -B build
cmake --build build
```

**Run the test suite:**

```bash
ctest --test-dir build
```

`tests/` has two independent suites, both discovered automatically by
the command above — no separate step needed for either: `tests/custom`
(this author's own framework, registered as one aggregate CTest entry)
and `tests/google` (GoogleTest, one CTest entry per `TEST()`, via
`gtest_discover_tests()`).

**Run benchmarks and check for regressions:**

```bash
./build/benchmarks
./build/regression                  # latest baseline vs. benchmarks/results/benchmark_results.json
./build/regression v1.2.0           # a specific baseline vs. current
./build/regression v1.2.0 v1.4.0    # two baselines against each other
```

Unlike the test suites, `benchmarks/google` (Google Benchmark) and
`regression/google` aren't CTest-integrated — build and run them
directly if you need them: `cmake --build build --target
google_benchmarks && ./build/google_benchmarks`.

`regression` picks the latest baseline by semantic version (`v1.10.0`
correctly outranks `v1.9.0`), not alphabetical filename order, and
auto-names its output (`regression_v1.2.0_vs_current.md`/`.json`, etc.).

See [packaging/README.md](packaging/README.md) for notes on verifying the vcpkg
port and Conan recipe locally, and [FUZZING.md](FUZZING.md) for running the
fuzz harnesses locally.

<div align="right"><a href="#-table-of-contents"><img src=".github/assets/back-to-top.svg" alt="Back to top" height="28"></a></div>

## <a id="benchmarks"></a>📊 Benchmarks

Unlike this author's other libraries, FalconHTTP has no natural drop-in
standard-library equivalent to benchmark against (there's no `std::`
HTTP parser, router, or serializer) — these are absolute measurements,
not a comparison. Full results across every subsystem and scale:
`benchmarks/results/v1_0_0.md`.

| Operation | FalconHTTP(1M) |
|---|---|
| FileCache::get() Existing Entry | 33.97 ms |
| FileCache::put() Fresh Key | 169.04 ms |
| Router::dispatch() Matching Route | 520.17 ms |
| Router::dispatch() No Matching Route | 241.79 ms |
| Router::matchStream() Matching Route | 427.60 ms |
| Router::matchStream() No Matching Route | 266.03 ms |
| PathMatcher::match() Two Params | 129.91 ms |
| Parse() Minimal Request Line | 377.05 ms |
| Parse() 50 Headers | 5.45 s |
| Serialize() 50 Headers | 650.93 ms |
| SseConnection::send() Framing + Write | 861.14 ms |
| 3-middleware Chain + Handler | 124.52 ms |
| UrlDecoder::decode() Mixed Encoding | 60.61 ms |

`Router::dispatch()`'s cost against the route table is the standout
finding: dispatching to the last-registered route is roughly **~142x
slower** going from a 10-route table to a 2000-route table (295.91 ms
vs. 41.93 s at 1M) — the direct, now-measured cost of `Router` scanning
routes linearly rather than through a trie/radix index. `matchStream()`
scans the same way and should be expected to show the same growth
curve on a large streaming route table, though this hasn't been
separately measured yet.

Somewhat less expected: parsing 50 headers costs roughly **8.4x more**
than serializing the same 50 (5.45 s vs. 650.93 ms at 1M), and the
per-header marginal cost actually grows with header count in the
Scaling data rather than staying flat — evidence pointing at `HashMap`
rehashing overhead compounding on the parse path, worth profiling
before assuming header parsing is as fast as it can go.

`SseConnection::send()`'s 861.14 ms/1M (framing plus an actual socket
write to a continuously-drained peer, not an isolated computation) is
roughly **1.7x** the cost of a full 3-middleware chain + handler
(124.52 ms) — the real, syscall-inclusive cost of pushing one event,
worth keeping in mind when picking an update interval for a
high-fanout streaming route.

<div align="right"><a href="#-table-of-contents"><img src=".github/assets/back-to-top.svg" alt="Back to top" height="28"></a></div>

## <a id="fuzzing"></a>🐛 Fuzzing

`HttpParser::parse()` and `PathMatcher::match()` are continuously
fuzzed via [ClusterFuzzLite](https://google.github.io/clusterfuzzlite/),
under AddressSanitizer and UndefinedBehaviorSanitizer. A short pass
runs on every PR touching either one's source; a longer pass runs
nightly.

Neither is a differential fuzzer — there's no shadow-model HTTP parser
or path matcher to compare against. `fuzz_http_parser` instead treats
`parse()`'s own documented `std::runtime_error` on malformed input as
an expected outcome and, on a successful parse, exercises every
`HttpRequest` accessor (including `json()` on the parsed body) so a
crash is the only finding that counts. `fuzz_path_matcher` fuzzes only
the path argument against a fixed, representative set of route
patterns — patterns are developer-written in real usage, the path is
what an attacker actually controls — and checks the one invariant
`PathMatcher.h` documents: a match means equal segment counts on both
sides.

`HttpSerializer`, `Router::dispatch()`/`matchStream()`, `SseConnection`,
and the built-in middleware are deliberately not fuzzed — all of them
operate on data FalconHTTP's own code already produced or already
validated, not on raw attacker-controlled bytes.

See [FUZZING.md](FUZZING.md) for running the harnesses locally and
reproducing a failing input.

<div align="right"><a href="#-table-of-contents"><img src=".github/assets/back-to-top.svg" alt="Back to top" height="28"></a></div>

## <a id="documentation"></a>📖 Documentation

Full API reference, generated with Doxygen from `docs/Doxyfile`:

**https://privateMwb.github.io/FalconHTTP/**

<div align="right"><a href="#-table-of-contents"><img src=".github/assets/back-to-top.svg" alt="Back to top" height="28"></a></div>

## <a id="contributing"></a>🤝 Contributing

Issues and pull requests are welcome. Before submitting a PR:

- Run the test suite (`ctest --test-dir build`)
- If you're changing a hot path, run `./build/regression` and mention
  the results in your PR description
- If you're changing `HttpParser` or `PathMatcher`, a quick local fuzz
  pass (see [FUZZING.md](FUZZING.md)) before pushing catches most
  malformed-input regressions before CI does

<div align="right"><a href="#-table-of-contents"><img src=".github/assets/back-to-top.svg" alt="Back to top" height="28"></a></div>

## <a id="changelog"></a>📝 Changelog

See the [Releases](https://github.com/privateMwb/FalconHTTP/releases)
page for version history and release notes.

<div align="right"><a href="#-table-of-contents"><img src=".github/assets/back-to-top.svg" alt="Back to top" height="28"></a></div>

## <a id="license"></a>📄 License

MIT — see [LICENSE](LICENSE) for details.

<p align="center">
  <sub>Built with C++23</sub>
</p>

<p align="center">
  <a href="#-table-of-contents"><img src=".github/assets/back-to-top.svg" alt="Back to top" height="28"></a>
</p>
