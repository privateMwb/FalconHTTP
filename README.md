# FalconHTTP

<p align="center">
  <img src="https://img.shields.io/github/v/release/privateMwb/FalconHTTP?style=for-the-badge&logo=github&color=yellow" alt="Version">
  <img src="https://img.shields.io/badge/License-MIT-orange?style=for-the-badge" alt="License - MIT">
  <img src="https://img.shields.io/badge/C%2B%2B-23-blue?style=for-the-badge&logo=c%2B%2B" alt="C++ - 23">
</p>

<p align="center">
  <a href="https://github.com/privateMwb/FalconHTTP/actions/workflows/build.yml">
    <img src="https://github.com/privateMwb/FalconHTTP/actions/workflows/build.yml/badge.svg" alt="Build and Test">
  </a>
  <a href="https://github.com/privateMwb/FalconHTTP/actions/workflows/benchmark.yml">
    <img src="https://github.com/privateMwb/FalconHTTP/actions/workflows/benchmark.yml/badge.svg" alt="Benchmarks">
  </a>
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
  <a href="https://github.com/privateMwb/FalconHTTP/actions/workflows/docs.yml">
    <img src="https://github.com/privateMwb/FalconHTTP/actions/workflows/docs.yml/badge.svg" alt="Documentation">
  </a>
  <a href="https://github.com/privateMwb/FalconHTTP/actions/workflows/release.yml">
    <img src="https://github.com/privateMwb/FalconHTTP/actions/workflows/release.yml/badge.svg" alt="Release">
  </a>
  <a href="https://github.com/privateMwb/FalconHTTP/actions/workflows/packaging.yml">
    <img src="https://github.com/privateMwb/FalconHTTP/actions/workflows/packaging.yml/badge.svg" alt="Packaging">
  </a>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/GCC-support-B46F1B?style=flat&logo=gnu" alt="GCC - support">
  <img src="https://img.shields.io/badge/Clang-support-045891?style=flat&logo=llvm" alt="Clang - support">
  <img src="https://img.shields.io/badge/MSVC-support-5C2D91?style=flat" alt="MSVC - support">
  <img src="https://img.shields.io/badge/AppleClang-support-000000?style=flat&logo=apple" alt="AppleClang - support">
</p>

FalconHTTP is an embedded C++ HTTP server library — thread-pool-backed request handling, pattern-based routing with path parameters, an onion-model middleware chain, and static file serving backed by an LRU cache, built entirely on this author's own container, caching, concurrency, JSON, and function-wrapper libraries.

## 📑 Table of Contents

- [Features](#features)
- [Requirements](#requirements)
- [Dependencies](#dependencies)
- [Installation](#installation)
- [Quick Start](#quick-start)
- [Project Structure](#project-structure)
- [Development](#development)
- [Benchmarks](#benchmarks)
- [Documentation](#documentation)
- [Contributing](#contributing)
- [Changelog](#changelog)
- [License](#license)

## <a id="features"></a>✨ Features

- **Case-insensitive header handling** — `HttpRequest`/`HttpResponse` normalize header names to lowercase on both insert and lookup (RFC 7230 §3.2), so `Content-Type` and `content-type` always resolve to the same entry.
- **Method-aware routing** — `Router::dispatch()` distinguishes a path with no matching route (`NotFound`/404) from a path that matches but not for the requested method (`MethodNotAllowed`/405), rather than collapsing both into a generic miss.
- **Onion-model middleware chain** — `Server::use()` registers middleware that wraps the handler in registration order, with `Cors`, `Logger`, and `Recovery` built in; `Recovery` converts any uncaught exception into a clean 500 instead of taking down the handling thread.
- **Request-size DoS protection** — `ServerConfig::maxHeaderSize`/`maxBodySize` reject an oversized header block or body with 431/413 before it's ever fully buffered into memory.
- **Static file serving with an LRU cache** — `StaticFileServer` resolves and serves files under a fixed root directory, with path-traversal protection and a `FileCache` avoiding repeated disk reads for hot files.
- **Move-only RAII sockets** — `Socket`/`Connection`/`Listener` can't have their underlying OS handle silently duplicated by an accidental copy.
- **Thread-pool-backed `Server`** — one thread pool per process, reused across every accepted connection rather than spawning an OS thread per request.

## <a id="requirements"></a>📋 Requirements

- A C++23-conformant compiler (tested: GCC, Clang, MSVC, AppleClang)
- CMake 3.20+
- Git submodules initialized — unlike this author's other, dependency-free libraries, FalconHTTP is a consumer of 6 of them (see [Dependencies](#dependencies)) and needs their source present to build from source

## <a id="dependencies"></a>🔗 Dependencies

FalconHTTP is built entirely on this author's own libraries, vendored as git submodules under `libs/internal/`:

| Library | Provides | Repository |
|---|---|---|
| VectorPro | `Vector<T>`, backing `Router::routes` and `Server`'s registered middleware list | [privateMwb/VectorPro](https://github.com/privateMwb/VectorPro) |
| JsonPro | `Json`, the body format for `HttpRequest::json()` / `HttpResponse::setJson()` | [privateMwb/JsonParser](https://github.com/privateMwb/JsonParser) |
| FunctionPro | `Function<>`, wrapping every `RouteHandler`, `MiddlewareFn`, and `NextHandler` callback | [privateMwb/FunctionPro](https://github.com/privateMwb/FunctionPro) |
| HashMapPro | `HashMap<K,V>`, backing header, query, and path parameter storage on every request and response | [privateMwb/HashMapPro](https://github.com/privateMwb/HashMapPro) |
| CachePro | `LRUCache<K,V>`, backing `FileCache`'s static file content cache | [privateMwb/LRUCache](https://github.com/privateMwb/LRUCache) |
| ThreadPoolPro | `ThreadPool`, driving `Server`'s per-connection thread pool | [privateMwb/ThreadPoolPro](https://github.com/privateMwb/ThreadPoolPro) |

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
│       │   ├── PathMatcher.h
│       │   └── Router.h
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
│   ├── support/
│   ├── suite/
│   ├── test_main.cpp
│   └── CMakeLists.txt
│
├── benchmarks/
│   ├── support/
│   ├── suite/
│   ├── baselines/
│   ├── bench_main.cpp
│   └── CMakeLists.txt
│
├── examples/
│   ├── support/
│   ├── suite/
│   ├── example_main.cpp
│   └── CMakeLists.txt
│
├── regression/
│   ├── support/
│   ├── regression_main.cpp
│   └── CMakeLists.txt
│
├── packaging/
│   ├── README.md
│   ├── recipes/
│   │   └── falconhttp/
│   ├── vcpkg/
│   │   └── ports/
│   │       └── falconhttp/
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
├── .gitignore
├── CMakeLists.txt
├── README.md
└── LICENSE
```

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

**Run benchmarks and check for regressions:**

```bash
./build/benchmarks
./build/regression                  # latest baseline vs. benchmarks/results/benchmark_results.json
./build/regression v1.2.0           # a specific baseline vs. current
./build/regression v1.2.0 v1.4.0    # two baselines against each other
```

`regression` picks the latest baseline by semantic version (`v1.10.0`
correctly outranks `v1.9.0`), not alphabetical filename order, and
auto-names its output (`regression_v1.2.0_vs_current.md`/`.json`, etc.).

See [packaging/README.md](packaging/README.md) for notes on verifying the vcpkg
port and Conan recipe locally.

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
| PathMatcher::match() Two Params | 129.91 ms |
| Parse() Minimal Request Line | 377.05 ms |
| Parse() 50 Headers | 5.45 s |
| Serialize() 50 Headers | 650.93 ms |
| 3-middleware Chain + Handler | 124.52 ms |
| UrlDecoder::decode() Mixed Encoding | 60.61 ms |

`Router::dispatch()`'s cost against the route table is the standout
finding: dispatching to the last-registered route is roughly **~142x
slower** going from a 10-route table to a 2000-route table (295.91 ms
vs. 41.93 s at 1M) — the direct, now-measured cost of `Router` scanning
routes linearly rather than through a trie/radix index.

Somewhat less expected: parsing 50 headers costs roughly **8.4x more**
than serializing the same 50 (5.45 s vs. 650.93 ms at 1M), and the
per-header marginal cost actually grows with header count in the
Scaling data rather than staying flat — evidence pointing at `HashMap`
rehashing overhead compounding on the parse path, worth profiling
before assuming header parsing is as fast as it can go.

## <a id="documentation"></a>📖 Documentation

Full API reference, generated with Doxygen from `docs/Doxyfile`:

**https://privateMwb.github.io/FalconHTTP/**

## <a id="contributing"></a>🤝 Contributing

Issues and pull requests are welcome. Before submitting a PR:

- Run the test suite (`ctest --test-dir build`)
- If you're changing a hot path, run `./build/regression` and mention
  the results in your PR description

## <a id="changelog"></a>📝 Changelog

See the [Releases](https://github.com/privateMwb/FalconHTTP/releases)
page for version history and release notes.

## <a id="license"></a>📄 License

MIT — see [LICENSE](LICENSE) for details.
