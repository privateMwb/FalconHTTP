# FalconHTTP

[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue)](https://en.cppreference.com/w/cpp/23)
[![Status](https://img.shields.io/badge/status-learning%20project-green)](https://github.com/privateMwb/FalconHTTP)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

A lightweight, multithreaded HTTP/1.1 server built completely from scratch in modern C++ — raw TCP sockets, a hand-written HTTP parser, a custom router, and a deliberately small middleware system, with no external networking or HTTP frameworks involved.

---

## Table of Contents

- [Overview](#overview)
- [Motivation / Goals](#motivation--goals)
- [Features](#features)
- [Design Overview](#design-overview)
- [Quick Example](#quick-example)
- [Core API](#core-api)
- [Project Structure](#project-structure)
- [Build Instructions](#build-instructions)
- [Notes](#notes)
- [License](#license)

---

## Overview

FalconHTTP is an HTTP/1.1 server implemented entirely by hand — every layer between accepting a raw TCP connection and sending back a complete HTTP response was built from scratch, with no web framework and no third-party networking or HTTP library involved.

It reuses the author's own library ecosystem from earlier projects — `HashMap`, `VectorPro`, `FunctionPro`, `LRUCache`, `JsonParser`, `PulseThreadPool` — rather than their C++ standard library equivalents, continuing the same "build your own tools" approach carried across this portfolio.

For the full design story — architecture, key decisions, real bugs found and fixed during testing, and platform-specific issues encountered along the way — see [`docs/Architecture.md`](docs/Architecture.md).

---

## Motivation / Goals

This project was built to understand:

- Low-level TCP/IP socket programming
- The HTTP/1.1 protocol, implemented by hand
- Building a scalable, multithreaded server from raw sockets
- Designing a clean routing system, including path parameters
- Serving static files safely (path traversal protection, MIME detection, caching)
- Parsing and generating JSON request/response bodies
- Designing a composable middleware system
- Practicing modular software architecture across a multi-layer system
- Producing a production-style portfolio project

---

## Features

### Networking
- TCP socket server with RAII-managed sockets
- Reliable connection I/O (guaranteed full-send, chunked receive)
- Multithreaded request handling via a thread pool

### HTTP Protocol
- HTTP/1.1 request parsing (method, path, version, headers, query parameters, body)
- HTTP response generation with automatic `Content-Length`
- Query string parsing with percent-decoding (`%XX`, `+` as space)
- `Content-Length`–aware body reading for requests larger than a single read

### Routing
- Express-style route registration (`get`, `post`, `put`, `del`)
- Path parameters (`/users/:id`) with automatic capture into the request
- First-match-wins dispatch in registration order

### Middleware
- Composable, wrapping ("onion") middleware model
- `Logger` — per-request timing and structured log lines
- `Cors` — configurable origin, automatic `OPTIONS` preflight handling
- `Recovery` — catches exceptions anywhere downstream and returns a safe `500`

### Static Files & JSON
- Static file serving with path-traversal protection (`std::filesystem`-based)
- MIME type detection by extension
- LRU-based file content caching
- JSON request/response bodies via `request.json()` / `response.setJson()`

### Testing
- Automated test suite covering parsing, routing, matching, decoding, serialization, static file serving, and middleware
- Verified end-to-end via real client requests (`curl`), including graceful shutdown

---

## Design Overview

A request flows through the system in a fixed sequence:

```
TCP Socket
   ↓
Connection (reliable send/receive)
   ↓
HttpParser (raw bytes → HttpRequest)
   ↓
Middleware Chain (Recovery → Logger → Cors → ...)
   ↓
Router (method + path matching, path params)
   ↓
Route Handler
   ↓
HttpSerializer (HttpResponse → raw bytes)
   ↓
Connection (send back to client)
```

Each accepted connection is dispatched onto a `ThreadPool` worker, so the accept loop itself stays simple and sequential while request handling runs concurrently.

Middleware uses a wrapping model — each middleware receives the rest of the chain as a callable and decides whether and when to call it — which is what allows `Recovery` to catch exceptions from everything downstream of it, and allows `Cors` to short-circuit the chain entirely for `OPTIONS` preflight requests.

See [`docs/Architecture.md`](docs/Architecture.md) for the full breakdown, including design tradeoffs and known limitations.

---

## Quick Example

```cpp
#include "Core/Server.h"
#include "Routing/Router.h"
#include "Middleware/Logger.h"
#include "Middleware/Cors.h"
#include "Middleware/Recovery.h"

using namespace FalconHTTP;

int main() {
	Routing::Router router;

	router.get("/", [](const HTTP::HttpRequest&, HTTP::HttpResponse& response) {
		response.setBody("Hello from FalconHTTP!");
	});

	router.get("/users/:id", [](const HTTP::HttpRequest& request, HTTP::HttpResponse& response) {
		response.setBody("User ID: " + request.pathParam("id"));
	});

	Core::Server server(router, 4);
	server.use(Middleware::Recovery{});
	server.use(Middleware::Logger{});
	server.use(Middleware::Cors{});

	server.start(8080);
	server.run();
}
```

---

## Core API

### Routing

```cpp
router.get(pattern, handler);
router.post(pattern, handler);
router.put(pattern, handler);
router.del(pattern, handler);
```

### Request / Response

```cpp
request.method();
request.path();
request.header(name);
request.queryParam(name);
request.pathParam(name);
request.json();

response.setStatus(status);
response.setHeader(name, value);
response.setBody(body);
response.setJson(json);
```

### Middleware

```cpp
server.use(Middleware::Recovery{});
server.use(Middleware::Logger{});
server.use(Middleware::Cors{"https://example.com"});
```

### Server

```cpp
Core::Server server(router, threadCount);
server.start(port);
server.run();
server.stop();
```

---

## Project Structure

```
FalconHTTP/
├── include/
│   ├── Core/
│   ├── HTTP/
│   ├── Routing/
│   ├── Middleware/
│   ├── FileServer/
│   ├── Utility/
│   └── Config/
│
├── src/
│   ├── Core/
│   ├── HTTP/
│   ├── Routing/
│   ├── Middleware/
│   ├── FileServer/
│   └── Utility/
│
├── examples/
│   └── basic_server.cpp
│
├── tests/
│
├── docs/
│   └── Architecture.md
│
├── libs/
│   ├── HashMap/
│   ├── VectorPro/
│   ├── JsonParser/
│   ├── FunctionPro/
│   ├── LRUCache/
│   └── PulseThreadPool/
│
├── CMakeLists.txt
└── README.md
```

---

## Build Instructions

### Requirements

- GCC 13+ (or any compiler with solid C++23 support)
- CMake 3.20+

### Build

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### Run the Example Server

```bash
./basic_server
```

Then, from another terminal:

```bash
curl http://localhost:8080/
curl http://localhost:8080/users/42
```

### Run Tests

```bash
./tests
```

---

## Notes

- This project intentionally implements HTTP/1.1 over `Content-Length`-declared bodies only — chunked transfer encoding, compression, and persistent connections are not implemented (see [`docs/Architecture.md`](docs/Architecture.md) for the full list of known limitations).
- Static file serving uses `std::filesystem` for path-traversal protection rather than hand-rolled string checks — one of the few deliberate uses of standard library facilities over a custom implementation, given the security sensitivity involved.
- `Server`, `Connection`, and `Socket` have no automated unit tests by design — they depend on real TCP I/O, which is verified through manual end-to-end testing instead. Every other component (parsing, routing, matching, decoding, serialization, static file serving, middleware) has automated test coverage.
- Graceful shutdown has been verified on Windows/MinGW; POSIX-documented behavior for the same scenario can vary by platform.

---

## License

[MIT](LICENSE) — free to use, modify, and distribute for educational and personal purposes.
