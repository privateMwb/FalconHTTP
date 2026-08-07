/**
 * @file            HttpParser.h
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
#include <FalconHTTP/HTTP/HttpRequest.h> // HttpRequest

#include <string_view> // std::string_view
// clang-format on

// Stateless parser for a complete, already-buffered HTTP/1.1 request.
// FalconHTTP does not stream-parse: Server reads the full header block
// plus (per Content-Length) the full body before calling parse() once.

namespace FalconHTTP::HTTP {

/**
 * @class HttpParser
 * @brief Stateless parser for a complete, already-buffered HTTP/1.1
 *        request.
 *
 * @details
 * FalconHTTP does not do incremental/streaming parsing: Server reads
 * bytes off the socket until it has the full header block plus (per
 * `Content-Length`) the full body, then calls parse() once on the
 * complete buffer.
 */
class HttpParser {
  public:
    // Core API

    /**
     * @brief Parses a complete HTTP/1.1 request.
     * @param raw The full request as received: request line, headers,
     *        blank line, and body concatenated. Everything remaining
     *        after the header block's terminating blank line is
     *        taken verbatim as the body - the parser does not
     *        truncate it to the declared `Content-Length`, so any
     *        trailing bytes beyond the body (e.g. from a future
     *        pipelined request) would currently be included.
     * @return The parsed HttpRequest.
     * @throws std::runtime_error if a line is missing its terminating
     *         CRLF, the request line doesn't have both a method and a
     *         version separated by spaces, or a header line has no
     *         colon. Malformed method tokens and unparseable path
     *         segments do NOT throw - see methodFromString() and
     *         parsePath() below.
     * @note The request path is stored exactly as received and is
     *       NOT percent-decoded (only query-string values are, via
     *       UrlDecoder). A path containing `%20` or `+` will not be
     *       decoded before routing/file lookup.
     */
    [[nodiscard]] static HttpRequest parse(std::string_view raw);

  private:
    // Private Helpers

    /// Extracts the next CRLF-terminated line from @p remaining and
    /// advances @p remaining past it.
    /// @throws std::runtime_error if no CRLF is found.
    [[nodiscard]] static std::string_view extractLine(std::string_view& remaining);

    /// Parses "METHOD path VERSION" into @p request's method, path,
    /// query params, and version.
    /// @throws std::runtime_error if the line doesn't contain two
    ///         space-separated fields.
    static void parseRequestLine(std::string_view line, HttpRequest& request);

    /// Parses a single "Name: value" header line into @p request.
    /// Leading spaces after the colon are trimmed; the header name is
    /// stored as-is (HttpRequest::setHeader() normalizes case).
    /// @throws std::runtime_error if the line has no colon.
    static void parseHeaderLine(std::string_view line, HttpRequest& request);

    /// Splits @p rawPath on the first `?` into the request path (set
    /// verbatim, undecoded) and a query string, then percent-decodes
    /// and stores each `key=value` (or valueless `key`) pair as a
    /// query parameter.
    static void parsePath(std::string_view rawPath, HttpRequest& request);
};

} // namespace FalconHTTP::HTTP