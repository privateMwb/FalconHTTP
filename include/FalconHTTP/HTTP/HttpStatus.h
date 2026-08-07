/**
 * @file            HttpStatus.h
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
#include <string_view> // std::string_view
// clang-format on

// HTTP response status code enumeration and reason-phrase lookup.

namespace FalconHTTP::HTTP {

/**
 * @enum HttpStatus
 * @brief Supported HTTP response status codes.
 * @note This set is intentionally minimal and does not yet include
 *       common codes such as the 3xx redirects (301/302/304/307/308),
 *       409 Conflict, 429 Too Many Requests, or the 5xx family beyond
 *       500/501. Extend as the library's feature set grows (e.g.
 *       redirects, rate limiting).
 */
enum class HttpStatus {
    Ok = 200,
    Created = 201,
    NoContent = 204,
    BadRequest = 400,
    Unauthorized = 401,
    Forbidden = 403,
    NotFound = 404,
    /// Returned by Router::dispatch() when a request's path matches
    /// a registered route but no handler is registered for its
    /// method (see Server::runChain()).
    MethodNotAllowed = 405,
    /// Returned by Server::handleConnection() when a request's body
    /// exceeds ServerConfig::maxBodySize (Content-Length rejected
    /// before or during the body read).
    PayloadTooLarge = 413,
    /// Returned by Server::handleConnection() when the request line
    /// plus header block exceeds ServerConfig::maxHeaderSize before
    /// the terminating CRLFCRLF was found.
    RequestHeaderFieldsTooLarge = 431,
    InternalServerError = 500,
    NotImplemented = 501
};

// Conversion

/**
 * @brief Looks up the standard reason phrase for a status code.
 * @return The reason phrase (e.g. "OK", "Not Found"), or "Unknown"
 *         for any value not in the HttpStatus enum.
 */
[[nodiscard]] std::string_view statusReasonPhrase(HttpStatus status) noexcept;

} // namespace FalconHTTP::HTTP