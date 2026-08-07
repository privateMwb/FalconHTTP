/**
 * @file            Connection.h
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

#include <cstddef> // std::size_t
#include <string>  // std::string
// clang-format on

// Higher-level, buffer-oriented I/O on top of a single Socket. Where
// Socket::send()/receive() may transfer fewer bytes than requested in a
// single call, Connection adds sendAll() (loops until the full buffer is
// sent or an error occurs) and receiveAvailable() (a single bounded read,
// sized for line/chunk-based HTTP parsing).

namespace FalconHTTP::Core {

/**
 * @class Connection
 * @brief Wraps a connected Socket with whole-buffer send/receive helpers.
 *
 * @details
 * Where Socket::send()/Socket::receive() may transfer fewer bytes than
 * requested in a single call, Connection provides sendAll() (loops
 * until the full buffer is sent or an error occurs) and
 * receiveAvailable() (a single bounded read, sized for line/chunk-based
 * HTTP parsing). Move-only, mirroring Socket.
 */
class Connection {

  public:
    // Identity

    /// Default number of bytes requested per receiveAvailable() call
    /// when the caller doesn't specify a size.
    static constexpr std::size_t DefaultReadChunk = 4096;

  private:
    // Storage
    Socket socket_;

  public:
    // Constructors & Destructor

    /// Constructs a Connection with no underlying socket (isOpen() == false).
    Connection() noexcept = default;

    /// Takes ownership of an already-connected @p socket (typically
    /// the result of Listener::accept()).
    explicit Connection(Socket socket) noexcept;

    /// Closes the underlying socket, if any.
    ~Connection() noexcept = default;

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    Connection(Connection&& other) noexcept = default;
    Connection& operator=(Connection&& other) noexcept = default;

    // Core API

    /**
     * @brief Writes exactly @p length bytes from @p data, looping
     *        over Socket::send() as needed until the full buffer has
     *        been written.
     * @return true if all @p length bytes were sent; false if the
     *         peer closed the connection or a socket error occurred
     *         partway through (in which case some prefix of the data
     *         may still have been delivered - Connection does not
     *         track/report how much).
     */
    [[nodiscard]] bool sendAll(const void* data, std::size_t length) noexcept;

    /**
     * @brief Performs a single read of up to @p chunkSize bytes.
     * @param chunkSize Maximum number of bytes to request from the
     *        socket in this call.
     * @return The bytes read (possibly fewer than @p chunkSize - this
     *         is one `recv()`, not a loop). Returns an empty string
     *         both when the peer closed the connection and when the
     *         underlying read returned 0 or an error; the two cases
     *         are not currently distinguishable from the return value
     *         alone.
     */
    [[nodiscard]] std::string receiveAvailable(std::size_t chunkSize = DefaultReadChunk) noexcept;

    /// Closes the underlying socket. Safe to call if already closed.
    void close() noexcept;

    /// @return true if the underlying socket currently owns a valid handle.
    [[nodiscard]] bool isOpen() const noexcept;
};

} // namespace FalconHTTP::Core