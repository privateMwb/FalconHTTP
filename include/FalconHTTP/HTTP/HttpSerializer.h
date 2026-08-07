/**
 * @file            HttpSerializer.h
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
#include <FalconHTTP/HTTP/HttpResponse.h> // HttpResponse

#include <string> // std::string
// clang-format on

// Stateless serializer producing a complete wire-format HTTP/1.1 response
// (status line, headers, blank line, body) from an HttpResponse.

namespace FalconHTTP::HTTP {
/**
 * @class HttpSerializer
 * @brief Stateless HTTP/1.1 response serializer.
 */
class HttpSerializer {
  public:
    // Core API

    /**
     * @brief Serializes @p response into a complete wire-format
     *        HTTP/1.1 response (status line, headers, blank line, body).
     * @return The serialized response, ready to write directly to a
     *         socket (e.g. via Connection::sendAll()).
     */
    [[nodiscard]] static std::string serialize(const HttpResponse& response);

  private:
    // Private Helpers

    /// Appends "HTTP/1.1 {code} {reason}\r\n".
    static void appendStatusLine(std::string& output, const HttpResponse& response);

    /**
     * @brief Appends all response headers plus the two headers
     *        FalconHTTP always computes itself.
     * @details Writes every header from response.headers() except
     *          "content-length" and "connection" (any user-set values
     *          for those are ignored), then unconditionally appends:
     *          - `Content-Length`, computed from response.body().size().
     *          - `Connection: close`, since FalconHTTP closes every
     *            connection after one response (no keep-alive support).
     */
    static void appendHeaders(std::string& output, const HttpResponse& response);
};

} // namespace FalconHTTP::HTTP