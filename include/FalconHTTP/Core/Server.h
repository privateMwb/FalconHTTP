/**
 * @file            Server.h
 *
 * @date            2026-5-8
 *
 * @version         1.0.0
 *
 * @copyright       Copyright (c) 2026 MWB
 *                  All rights reserved.
 *                  https://github.com/privateMwb/FalconHTTP
 *
 * @attention       This source is released under the MIT license
 *                  SPDX-License-Identifier: MIT
 *                  <http://opensource.org/licenses/MIT>
 */

#pragma once

// clang-format off
#include <FalconHTTP/Core/Connection.h>       // Connection
#include <FalconHTTP/Core/Listener.h>         // Listener
#include <FalconHTTP/Routing/Router.h>        // Router, DispatchResult
#include <FalconHTTP/Middleware/Middleware.h> // MiddlewareFn
#include <FalconHTTP/Config/ServerConfig.h>   // ServerConfig

#include <ThreadPoolPro/ThreadPool.h> // ThreadPool
#include <VectorPro/Vector.h>         // Vector

#include <cstdint> // uint16_t
#include <atomic>  // std::atomic
// clang-format on

using namespace ThreadPoolPro;
using namespace VectorPro;

// Top-level HTTP server: owns a Listener and a thread pool, and drives
// the full request/response lifecycle per connection (thread-per-
// connection via the pool, no async/event-loop I/O, no keep-alive).

namespace FalconHTTP::Core {

/**
 * @class Server
 * @brief Top-level HTTP server: owns a Listener and a thread pool,
 *        and drives the full request/response lifecycle per connection.
 *
 * @details
 * ## Connection model
 * Thread-per-connection via a pool: run() blocks in a loop calling
 * Listener::accept(), and dispatches each accepted connection to
 * pool_ via `detach()`. There is no async/event-loop I/O (no
 * epoll/kqueue/io_uring) - each pooled thread blocks synchronously on
 * that connection's reads/writes in handleConnection().
 *
 * ## Per-request flow (handleConnection())
 * 1. Read until the header block's terminating CRLFCRLF is seen,
 *    enforcing maxHeaderSize_.
 * 2. Scan headers for `Content-Length` (case-insensitively) without a
 *    full parse.
 * 3. Enforce maxBodySize_ against the declared Content-Length.
 * 4. Read further chunks until the full body has arrived.
 * 5. HttpParser::parse() the complete buffer, run runChain(), then
 *    HttpSerializer::serialize() and send the response.
 * 6. Close the connection - **no keep-alive**: every connection is
 *    closed after exactly one response, regardless of what the
 *    request's `Connection` header requested. (HttpSerializer emits
 *    `Connection: close` on every response to keep this honest to
 *    the client.)
 *
 * Any `std::exception` escaping this flow (e.g. from HttpParser or a
 * handler not wrapped in Recovery middleware) is caught here and
 * converted to a 500 response - this is a second, server-level safety
 * net in addition to (not a replacement for) the Recovery middleware.
 *
 * ## Configuration
 * Server(Router&, const Config::ServerConfig&) reads `threadCount`,
 * `port`, `maxHeaderSize`, and `maxBodySize` from the given
 * ServerConfig. `staticRoot`, `fileCacheCapacity`, and
 * `corsAllowedOrigin` are NOT read by Server - it does not own a
 * StaticFileServer or Cors instance - construct those directly from
 * the same config object (see ServerConfig.h).
 *
 * @note Server() (the default constructor) leaves the router
 *       unbound; run() checks for this and returns immediately
 *       without accepting any connections. Use one of the
 *       parameterized constructors for a functional server.
 * @note ThreadPool (ThreadPoolPro) is an external dependency; its
 *       default-construction behavior when Server's default
 *       constructor is used is not verified here - consult
 *       ThreadPoolPro's own documentation.
 */
class Server {
  public:
    // Identity

    /// Bytes requested per Connection::receiveAvailable() call while
    /// reading a request.
    static constexpr std::size_t DefaultReadChunk = 4096;

    /// Default value for maxHeaderSize_ when a Server is constructed
    /// without an explicit ServerConfig (see the threadCount-only
    /// constructor). Matches ServerConfig::maxHeaderSize's default.
    static constexpr std::size_t DefaultMaxHeaderSize = 16 * 1024; // 16 KiB

    /// Default value for maxBodySize_ when a Server is constructed
    /// without an explicit ServerConfig. Matches
    /// ServerConfig::maxBodySize's default.
    static constexpr std::size_t DefaultMaxBodySize = 10 * 1024 * 1024; // 10 MiB

  private:
    // Storage
    Listener listener_;
    Routing::Router* router_ = nullptr;
    ThreadPool pool_;
    Vector<Middleware::MiddlewareFn> middleware_;
    std::atomic<bool> running_{false};

    /// Port used by the no-argument start() overload. Defaults to
    /// ServerConfig's default port (8080) even when Server was built
    /// via the threadCount-only constructor, so start() has a
    /// sensible fallback either way; start(uint16_t) always
    /// overrides it explicitly regardless of how Server was built.
    uint16_t configuredPort_ = 8080;

    /// Active request line + header block size cap. See
    /// class-level "Per-request flow" and ServerConfig::maxHeaderSize.
    std::size_t maxHeaderSize_ = DefaultMaxHeaderSize;

    /// Active request body size cap. See class-level "Per-request
    /// flow" and ServerConfig::maxBodySize.
    std::size_t maxBodySize_ = DefaultMaxBodySize;

  public:
    // Constructors

    /// Constructs a Server with no router bound. run() will be a
    /// no-op until reconstructed via a parameterized constructor
    /// (Server has no way to bind a router after construction).
    Server() noexcept = default;

    /**
     * @brief Constructs a Server with explicit thread count and
     *        default DoS caps (DefaultMaxHeaderSize/DefaultMaxBodySize)
     *        and default port (8080, used only by the no-arg start()
     *        overload).
     * @param router Router used to dispatch matched requests. Stored
     *        as a non-owning pointer - @p router must outlive this
     *        Server; Server does not extend its lifetime.
     * @param threadCount Number of worker threads in the connection
     *        pool.
     */
    explicit Server(Routing::Router& router, std::size_t threadCount) noexcept;

    /**
     * @brief Constructs a Server from a ServerConfig, reading
     *        threadCount, port, maxHeaderSize, and maxBodySize from it.
     * @param router Router used to dispatch matched requests. Stored
     *        as a non-owning pointer - @p router must outlive this
     *        Server; Server does not extend its lifetime.
     * @param config Read once at construction time (not stored or
     *        watched for later changes - mutating @p config after
     *        this call has no effect on this Server).
     */
    explicit Server(Routing::Router& router, const Config::ServerConfig& config) noexcept;

    ~Server() noexcept = default;

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    // Middleware Registration

    /// Registers @p middleware to run on every request, in
    /// registration order (onion model - see Middleware.h).
    void use(Middleware::MiddlewareFn middleware);

    // Lifecycle

    /// Binds and starts listening on @p port. Also updates the port
    /// used by the no-argument start() overload, if called later.
    /// @return true on success; see Listener::start() for failure causes.
    [[nodiscard]] bool start(uint16_t port);

    /// Binds and starts listening on configuredPort_ - the port from
    /// the ServerConfig this Server was constructed with, or 8080 if
    /// it was not constructed from a ServerConfig and start(uint16_t)
    /// has not been called yet.
    /// @return true on success; see Listener::start() for failure causes.
    [[nodiscard]] bool start();

    /**
     * @brief Blocks, accepting and handling connections until stop()
     *        is called from another thread.
     * @details No-op (returns immediately) if no router is bound or
     *          start() has not succeeded. Each accepted connection is
     *          dispatched to the thread pool; a failed accept()
     *          (invalid socket) is silently skipped and the loop
     *          continues.
     */
    void run();

    /// Signals run()'s loop to stop after its current accept() call
    /// returns, and stops the listener. Does not forcibly interrupt
    /// in-flight handleConnection() calls already running on the pool.
    void stop() noexcept;

    /// @return true between a run() call starting and stop() being called.
    [[nodiscard]] bool isRunning() const noexcept;

  private:
    // Private Helpers

    /// Reads, parses, dispatches, and responds to a single request on
    /// @p connection, then closes it. See class-level docs for the
    /// full per-request flow. Runs on a pool thread.
    void handleConnection(Connection connection);

    /// Recursively invokes middleware_[index], passing a NextHandler
    /// that calls runChain(index + 1, ...). At index == middleware_.size(),
    /// invokes router_->dispatch() and translates its DispatchResult
    /// into a 404/405 response if no route matched.
    void runChain(std::size_t index, HTTP::HttpRequest& request,
                  HTTP::HttpResponse& response) const;
};

} // namespace FalconHTTP::Core