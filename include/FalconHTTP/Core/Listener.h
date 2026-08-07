/**
 * @file            Listener.h
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
#include <FalconHTTP/Core/Socket.h> // Socket

#include <cstdint> // uint16_t
// clang-format on

// Binds to a port and accepts incoming TCP connections. IPv4 only, since
// the underlying Socket is created with AF_INET.

namespace FalconHTTP::Core {

/**
 * @class Listener
 * @brief Binds to a port and accepts incoming TCP connections.
 *
 * @details
 * Wraps the bind/listen/accept sequence over a single Socket. Move-only
 * (mirrors Socket's ownership model). Not thread-safe: a single
 * Listener is intended to be driven by one accept loop (see
 * Server::run()).
 *
 * @note IPv4 only, since the underlying Socket is created with
 *       `AF_INET` (see Socket::createTcp()).
 */
class Listener {
  public:
    // Identity

    /// Default pending-connection queue length passed to `::listen()`
    /// when the caller doesn't specify one.
    static constexpr int defaultBacklog = 128;

  private:
    // Storage
    Socket socket_;
    uint16_t port_ = 0;

  public:
    // Constructors & Destructor

    /// Constructs a Listener that is not yet listening (isListening() == false).
    Listener() noexcept = default;

    /// Stops listening and releases the underlying socket, if any.
    ~Listener() noexcept = default;

    Listener(const Listener&) = delete;
    Listener& operator=(const Listener&) = delete;

    Listener(Listener&& other) noexcept = default;
    Listener& operator=(Listener&& other) noexcept = default;

    // Core API

    /**
     * @brief Creates a TCP socket, binds it to @p port on all
     *        interfaces (`INADDR_ANY`), and starts listening.
     * @param port TCP port to bind to.
     * @param backlog Pending-connection queue length for `::listen()`.
     * @return true on success; false if socket creation, `SO_REUSEADDR`,
     *         `bind()`, or `listen()` failed. On failure the internal
     *         socket is closed and the Listener remains not-listening.
     */
    [[nodiscard]] bool start(uint16_t port, int backlog = defaultBacklog) noexcept;

    /**
     * @brief Blocks until a client connects, then accepts it.
     * @return A Socket owning the new connection's handle. If
     *         `::accept()` fails, the returned Socket has
     *         isValid() == false - callers must check before use
     *         (see Server::run()).
     */
    [[nodiscard]] Socket accept() noexcept;

    /// Closes the listening socket and resets port() to 0. Safe to
    /// call even if not currently listening.
    void stop() noexcept;

    /// @return true if the listening socket is currently open.
    [[nodiscard]] bool isListening() const noexcept;

    /// @return The port passed to the last successful start() call,
    ///         or 0 if not currently listening.
    [[nodiscard]] uint16_t port() const noexcept;
};

} // namespace FalconHTTP::Core