/**
 * @file            SseConnection.h
 *
 * @date            2026-9-2
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
#include <FalconHTTP/Core/Connection.h> // Connection

#include <HashMapPro/HashMap.h> // HashMap

#include <string>      // std::string
#include <string_view> // std::string_view
// clang-format on

// A long-lived, repeatedly-writable wrapper around a single Connection,
// for a Stream-kind route's handler to push Server-Sent Events over.
// Unlike the rest of FalconHTTP's request/response flow, one
// SseConnection is expected to be written to many times over its
// lifetime rather than populating a single HttpResponse.

namespace FalconHTTP::Streaming {

/**
 * @class SseConnection
 * @brief Wraps a Core::Connection, sending the SSE preamble once on
 *        construction and framing subsequent send() calls as SSE
 *        events.
 *
 * @details
 * Move-only, mirroring Connection. Once the preamble write or any
 * send() fails (peer disconnected, socket error), the connection is
 * marked dead and every subsequent send() is a no-op returning false -
 * callers (Server, and the StreamHandler itself) should check
 * isConnected() to stop producing events for a client that's gone
 * rather than continuing to call a permanently-failing send().
 *
 * SseConnection does not itself poll for the peer disconnecting
 * between send() calls (Connection has no such primitive) - liveness
 * is only actually discovered on the next attempted write.
 */
class SseConnection {

  private:
    // Storage
    Core::Connection connection_;
    bool alive_ = false;

  public:
    // Constructors & Destructor

    /**
     * @brief Takes ownership of @p connection and immediately writes
     *        the SSE preamble (`200 OK`, `Content-Type:
     *        text/event-stream`, no `Content-Length`) over it.
     * @param extraHeaders Additional response headers to fold into
     *        the preamble - e.g. CORS headers a middleware already
     *        set on the matched route's HttpResponse before the
     *        connection was upgraded (see Server::handleConnection()
     *        for how these are collected; SseConnection itself has no
     *        knowledge of middleware or HttpResponse). Each entry is
     *        written verbatim as `key: value\r\n`; callers are
     *        responsible for not passing hop-by-hop or framing
     *        headers (Content-Length, Transfer-Encoding, Connection)
     *        that would conflict with the preamble's own.
     * @details If the preamble write fails, isConnected() reports
     *          false immediately and every send() thereafter is a
     *          no-op - construction itself never throws or reports
     *          failure other than through isConnected().
     */
    explicit SseConnection(
        Core::Connection connection,
        const HashMapPro::HashMap<std::string, std::string>& extraHeaders =
            HashMapPro::HashMap<std::string, std::string>()) noexcept;

    /// Closes the underlying connection, if still open.
    ~SseConnection() noexcept = default;

    SseConnection(const SseConnection&) = delete;
    SseConnection& operator=(const SseConnection&) = delete;

    SseConnection(SseConnection&& other) noexcept = default;
    SseConnection& operator=(SseConnection&& other) noexcept = default;

    // Core API

    /**
     * @brief Frames and sends one SSE event.
     * @param event The event's `event:` field. Skipped entirely
     *        (matching vanilla SSE's unnamed-event default) if empty.
     * @param data The event's payload. Split on '\n' and emitted as
     *        one `data:` line per line, per the SSE multi-line-data
     *        convention - a literal embedded newline in @p data does
     *        not terminate the event early.
     * @return true if the event was written successfully; false if
     *         the connection was already dead (preamble or a prior
     *         send() failed) or this write itself failed, in either
     *         case marking the connection dead for all future calls.
     */
    [[nodiscard]] bool send(std::string_view event, std::string_view data) noexcept;

    /// @return false once the preamble or any send() has failed;
    /// true otherwise. Does not perform a new write to check -
    /// reflects only the outcome of the last attempted write.
    [[nodiscard]] bool isConnected() const noexcept;

    /// Closes the underlying connection early and marks it dead, so a
    /// handler can end the stream deliberately (as opposed to being
    /// ended by the peer). Safe to call if already dead.
    void close() noexcept;

  private:
    // Private Helpers

    /// Writes the initial status-line/headers preamble. Called once,
    /// from the constructor.
    bool sendPreamble(const HashMapPro::HashMap<std::string, std::string>& extraHeaders) noexcept;
};

} // namespace FalconHTTP::Streaming
