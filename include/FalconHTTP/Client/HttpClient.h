/**
 * @file            HttpClient.h
 *
 * @date            2026-10-6
 *
 * @version         1.0.0
 *
 * @copyright       Copyright (c) 2026 privateMwb
 *                  All rights reserved.
 *                  https://github.com/privateMwb/FalconHTTP
 *
 * @attention       This source is released under the MIT license
 *                  SPDX-License-Identifier: MIT
 *                  <http://opensource.org/licenses/MIT>
 */

#pragma once

// clang-format off
#include <FalconHTTP/HTTP/HttpMethod.h> // HttpMethod - request method
#include <HashMapPro/HashMap.h>         // HashMap - request headers in, response headers out
#include <chrono>      // std::chrono::milliseconds - connect/IO timeouts
#include <cstddef>     // std::size_t - maxResponseSize
#include <cstdint>     // uint16_t - destination port
#include <optional>    // std::optional - "no response" on any transport failure
#include <string>      // std::string - host, path, body
// clang-format on

// Minimal blocking HTTP/1.1 client, the outbound counterpart to Server.
// One request per connection (`Connection: close`), matching what Server
// itself speaks: no keep-alive, no chunked encoding, no redirects
// followed, no TLS, IPv4 dotted-decimal hosts only.

namespace FalconHTTP::Client {

/// @brief A parsed HTTP response. Header names are lowercase.
struct ClientResponse {
    int statusCode = 0;
    HashMapPro::HashMap<std::string, std::string> headers;
    std::string body;

    /// @return true for any 2xx status.
    [[nodiscard]] bool ok() const noexcept {
        return statusCode >= 200 && statusCode < 300;
    }
};

/// @brief Per-client limits. Every wait is bounded; nothing blocks forever.
struct ClientOptions {
    /// Maximum time to establish the TCP connection.
    std::chrono::milliseconds connectTimeout{500};
    /// Maximum wait for each individual send()/receive() call, not the
    /// whole exchange.
    std::chrono::milliseconds ioTimeout{1000};
    /// Responses whose headers plus body exceed this are abandoned.
    std::size_t maxResponseSize = 10 * 1024 * 1024;
};

/**
 * @class HttpClient
 * @brief Sends one HTTP request per call over a fresh TCP connection.
 *
 * @details
 * Stateless apart from its options, so one instance may be shared by
 * any number of threads: every send() uses its own local Socket.
 * Any failure to complete a well-formed exchange - unreachable host,
 * refused connection, timeout, malformed or oversized response, or the
 * peer closing before Content-Length bytes arrived - yields
 * `std::nullopt`. A response with an error status (404, 503, ...) is
 * still a response and is returned normally; use ClientResponse::ok().
 */
class HttpClient {
  private:
    ClientOptions options_;

  public:
    /// @param options Timeouts and size cap; defaults suit LAN/loopback RPC.
    explicit HttpClient(ClientOptions options = ClientOptions()) noexcept;

    /**
     * @brief Sends a request and reads the full response.
     *
     * @param method Request method.
     * @param host IPv4 address in dotted-decimal notation (not a hostname).
     * @param port Destination TCP port.
     * @param path Request target, e.g. `"/raft/append?x=1"`. Must start with '/'.
     * @param body Request body; Content-Length is computed from it.
     * @param headers Extra request headers. `host`, `connection`, and
     *        `content-length` are ignored if present (set by the client).
     *
     * @return The response, or `std::nullopt` on any failure (see class docs).
     */
    [[nodiscard]] std::optional<ClientResponse>
    send(HTTP::HttpMethod method, const std::string& host, uint16_t port, const std::string& path,
         const std::string& body = std::string(),
         const HashMapPro::HashMap<std::string, std::string>& headers =
             HashMapPro::HashMap<std::string, std::string>()) const;

    /// @brief send() with HttpMethod::Get and no body.
    [[nodiscard]] std::optional<ClientResponse> get(const std::string& host, uint16_t port,
                                                    const std::string& path) const;

    /// @brief send() with HttpMethod::Post.
    [[nodiscard]] std::optional<ClientResponse>
    post(const std::string& host, uint16_t port, const std::string& path, const std::string& body,
         const HashMapPro::HashMap<std::string, std::string>& headers =
             HashMapPro::HashMap<std::string, std::string>()) const;

    /// @brief send() with HttpMethod::Put.
    [[nodiscard]] std::optional<ClientResponse>
    put(const std::string& host, uint16_t port, const std::string& path, const std::string& body,
        const HashMapPro::HashMap<std::string, std::string>& headers =
            HashMapPro::HashMap<std::string, std::string>()) const;

    /// @brief send() with HttpMethod::Delete and no body.
    [[nodiscard]] std::optional<ClientResponse> del(const std::string& host, uint16_t port,
                                                    const std::string& path) const;
};

} // namespace FalconHTTP::Client
