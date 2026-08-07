# Example Suite

This document describes the example categories under `suite/` — what
each one demonstrates, and the individual example files it contains.

Unlike the test suite, an example doesn't assert correctness — it
demonstrates real usage of the library, including deliberate misuse
where instructive (see Misuse), so the reader sees both the correct
pattern and the mistake it guards against.

Every example file ends with `REGISTER_EXAMPLE_SUITE()`, which derives
the suite's category from its containing directory and assigns it a
sequential id within that category. This applies uniformly across
every category below.

---

## Advanced

Demonstrates deeper mechanics of the library — the server lifecycle,
thread pool sizing, static-file cache staleness, and sharing one
ServerConfig across every piece that reads it.

### Examples

- `graceful_shutdown.cpp` — isRunning() across the lifecycle, stopping a blocking run() from another thread, and what stop() does and doesn't interrupt
- `thread_pool_sizing.cpp` — setting threadCount directly or via ServerConfig, and why it's a hard ceiling on concurrent connections
- `file_cache_tuning.cpp` — choosing a cache capacity, the staleness gap when a cached file is edited on disk, and manual invalidation via a shared FileCache
- `server_config.cpp` — ServerConfig's defaults, which fields Server itself reads vs. passes through, and building Server/StaticFileServer/Cors from one shared config

---

## Integration

Demonstrates interoperability with the rest of a codebase — embedding
the router inside a domain-specific class, wiring Logger and Recovery
into real request handling, and combining static files with API
routes.

### Examples

- `router_embedding.cpp` — wrapping Router as a private implementation detail behind a domain-specific API
- `logger_integration.cpp` — Logger wired in front of real route dispatch, logging the final response status whether a handler or the router itself produced it
- `recovery_middleware.cpp` — Recovery catching a handler's exception vs. the same handler with no Recovery in the chain
- `static_plus_router.cpp` — a fallback middleware that serves static files first and only continues to API routing on a miss

---

## Misuse

Demonstrates common mistakes and the errors or unexpected behavior
they lead to, alongside the correct pattern.

### Examples

- `oversized_headers.cpp` — a request whose header block exceeds maxHeaderSize, rejected with 431 Request Header Fields Too Large
- `oversized_body.cpp` — a Content-Length exceeding maxBodySize, rejected with 413 Payload Too Large before the body is read
- `unbound_router.cpp` — a default-constructed Server's run() silently doing nothing because no router was ever bound
- `malformed_request.cpp` — HttpParser::parse() throwing on malformed input, and why an uncaught throw becomes a generic 500 instead of a 400

---

## Patterns

Demonstrates common usage idioms built on top of the core API — path
and query params, the onion-model middleware chain, writing custom
middleware, and configuring CORS.

### Examples

- `param_routes.cpp` — multiple :param segments in one pattern, path params vs. query params, and converting a param to another type
- `middleware_chaining.cpp` — registration order vs. execution order in the onion-model chain, and a middleware short-circuiting by not calling next()
- `custom_middleware.cpp` — writing a MiddlewareFn lambda, and composing a custom middleware with the built-in Recovery
- `cors_setup.cpp` — Cors's default (allow all origins) vs. a restricted origin, and the OPTIONS preflight short-circuit

---

## Quickstart

Demonstrates fundamental, everyday usage — registering routes,
shaping a response, serving static files, and starting a server.

### Examples

- `basic_server.cpp` — registering routes, constructing a Server from ServerConfig, registering middleware, and starting/running the server
- `first_route.cpp` — registering a route with a path param and dispatching a request directly against the Router
- `sending_response.cpp` — setting a status, body, and headers on HttpResponse, plus a JSON body via setJson()
- `static_files.cpp` — serving an existing file with StaticFileServer, a missing-file case, and wiring it into a Router route
