/**
 * @file            HttpRequest.h
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
#include <FalconHTTP/HTTP/HttpMethod.h> // HttpMethod

#include <HashMapPro/HashMap.h> // HashMapPro
#include <JsonPro/Json.h>       // Json

#include <string> // std::string
// clang-format on

using namespace HashMapPro;
using namespace JsonPro;

// Represents a parsed incoming HTTP request: method, path, headers,
// query/path parameters, and body. Populated by HttpParser::parse() and
// Router::dispatch() (path params), then passed through the middleware
// chain and into the matched route handler.

namespace FalconHTTP::HTTP {

/**
 * @class HttpRequest
 * @brief Mutable container for a request's method, path, headers,
 *        query/path parameters, and body.
 *
 * @details
 * Populated by HttpParser::parse() and Router::dispatch() (path
 * params), then passed through the middleware chain and into the
 * matched route handler.
 */
class HttpRequest {
  public:
    // Identity
    static constexpr std::string_view defaultVersion = "HTTP/1.1";

  private:
    // Storage
    HttpMethod method_ = HttpMethod::Unknown;
    std::string path_;
    std::string version_;
    HashMap<std::string, std::string> headers_;
    HashMap<std::string, std::string> queryParams_;
    HashMap<std::string, std::string> pathParams_;
    std::string body_;

  public:
    // Constructor
    HttpRequest() noexcept = default;

    // Mutators
    void setMethod(HttpMethod method) noexcept;
    void setPath(std::string path) noexcept;
    void setVersion(std::string version) noexcept;

    /// @param name Header name. Stored case-insensitively (normalized
    ///        to lowercase internally, per RFC 7230 §3.2) - setting
    ///        "Content-Type" and "content-type" updates the same entry.
    void setHeader(std::string name, std::string value);

    /// @param name Query parameter name. Matched case-sensitively -
    ///        unlike headers, query keys are not normalized.
    void setQueryParam(std::string name, std::string value);

    /// @param name Path parameter name (e.g. "id" for pattern "/users/:id").
    ///        Matched case-sensitively. Populated by Router::dispatch().
    void setPathParam(std::string name, std::string value);

    void setBody(std::string body) noexcept;

    // Accessors
    [[nodiscard]] HttpMethod method() const noexcept;
    [[nodiscard]] const std::string& path() const noexcept;
    [[nodiscard]] const std::string& version() const noexcept;
    [[nodiscard]] const std::string& body() const noexcept;

    /// @param name Header name, matched case-insensitively.
    /// @note Declared noexcept but performs a string copy/allocation
    ///       internally to normalize case; an allocation failure here
    ///       would terminate rather than throw, same as header() below.
    [[nodiscard]] bool hasHeader(const std::string& name) const noexcept;

    /// @param name Header name, matched case-insensitively.
    /// @return The header's value, or an empty string if absent.
    ///         Note this is ambiguous with a header explicitly set to
    ///         an empty value - use hasHeader() first to distinguish.
    [[nodiscard]] std::string header(const std::string& name) const noexcept;

    /// @param name Query parameter name, matched case-sensitively.
    [[nodiscard]] bool hasQueryParam(const std::string& name) const noexcept;

    /// @return The query parameter's value, or an empty string if
    ///         absent (see header()'s note on this ambiguity).
    [[nodiscard]] std::string queryParam(const std::string& name) const noexcept;

    /// @param name Path parameter name, matched case-sensitively.
    [[nodiscard]] bool hasPathParam(const std::string& name) const noexcept;

    /// @return The path parameter's value, or an empty string if
    ///         absent (see header()'s note on this ambiguity).
    [[nodiscard]] std::string pathParam(const std::string& name) const noexcept;

    // JSON

    /**
     * @brief Parses the request body as JSON.
     * @return The parsed Json value.
     * @throws Whatever JsonPro::Json::parse() throws on malformed
     *         input - this is an external-library contract that
     *         FalconHTTP does not itself validate or catch; callers
     *         should consult JsonPro's documentation for its exact
     *         error-signaling behavior (exception type, or a
     *         null/invalid Json result) before relying on this.
     */
    [[nodiscard]] Json json() const;
};

} // namespace FalconHTTP::HTTP