/**
 * @file            HttpResponse.h
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
#include <FalconHTTP/HTTP/HttpStatus.h> // HttpStatus

#include <HashMapPro/HashMap.h> // HashMapPro
#include <JsonPro/Json.h>       // Json

#include <string> // std::string
// clang-format on

using namespace HashMapPro;
using namespace JsonPro;

// Mutable container for an outgoing HTTP response's status, headers, and
// body, built up by middleware and the matched route handler. Content-
// Length and Connection are computed by HttpSerializer at serialization
// time; setHeader() values for those two names are ignored on output.

namespace FalconHTTP::HTTP {
/**
 * @class HttpResponse
 * @brief Mutable container for a response's status, headers, and body.
 *
 * @details
 * Built up by middleware (outer-to-inner then inner-to-outer, per the
 * onion model - see Middleware.h) and the matched route handler, then
 * finalized by HttpSerializer::serialize(). `Content-Length` and
 * `Connection` are computed by HttpSerializer at serialization time -
 * any value set here via setHeader() for those two names is ignored
 * on output (see HttpSerializer::appendHeaders()).
 */
class HttpResponse {
  private:
    // Storage
    HttpStatus status_ = HttpStatus::Ok;
    HashMap<std::string, std::string> headers_;
    std::string body_;

  public:
    // Constructors

    /// Constructs a response defaulting to HttpStatus::Ok.
    HttpResponse() noexcept = default;

    /// Constructs a response with the given initial status.
    explicit HttpResponse(HttpStatus status) noexcept;

    // Mutators
    void setStatus(HttpStatus status) noexcept;

    /// @param name Header name. Stored case-insensitively (normalized
    ///        to lowercase internally, per RFC 7230 §3.2). Setting
    ///        "content-length" or "connection" here has no effect on
    ///        the serialized output - see class-level note.
    void setHeader(std::string name, std::string value);

    void setBody(std::string body) noexcept;

    /**
     * @brief Serializes @p json into the body and sets
     *        `Content-Type: application/json`.
     * @throws Whatever JsonPro::Json::dump() throws - an external-
     *         library contract not validated by FalconHTTP itself.
     */
    void setJson(const Json& json);

    // Accessors
    [[nodiscard]] HttpStatus status() const noexcept;
    [[nodiscard]] const std::string& body() const noexcept;

    /// @param name Header name, matched case-insensitively.
    /// @note Declared noexcept but performs a string copy/allocation
    ///       internally to normalize case (see HttpRequest::hasHeader()
    ///       for the same caveat).
    [[nodiscard]] bool hasHeader(const std::string& name) const noexcept;

    /// @param name Header name, matched case-insensitively.
    /// @return The header's value, or an empty string if absent (this
    ///         is ambiguous with a header explicitly set to an empty
    ///         value - use hasHeader() first to distinguish).
    [[nodiscard]] std::string header(const std::string& name) const noexcept;

    /// @return All headers as set via setHeader(), keyed by their
    ///         lowercase-normalized names. Used directly by
    ///         HttpSerializer to build the wire response.
    [[nodiscard]] const HashMap<std::string, std::string>& headers() const noexcept;
};

} // namespace FalconHTTP::HTTP