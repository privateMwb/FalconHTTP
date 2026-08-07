/**
 * @file            Route.h
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
#include <FalconHTTP/HTTP/HttpMethod.h>       // HttpMethod
#include <FalconHTTP/Routing/RouteHandler.h> // RouteHandler

#include <string> // std::string
// clang-format on

// A single registered (method, pattern, handler) binding held in
// Router::routes.

namespace FalconHTTP::Routing {

/**
 * @class Route
 * @brief One entry in Router::routes: which method and path pattern
 *        invoke which handler.
 */
class Route {
  public:
    // Identity
    HTTP::HttpMethod method = HTTP::HttpMethod::Unknown;
    std::string pattern;
    RouteHandler handler;

    // Constructors

    /// Constructs an empty/unbound Route (method Unknown, empty
    /// pattern and handler).
    Route() noexcept = default;

    /// @param method HTTP method this route responds to.
    /// @param pattern Path pattern (see PathMatcher for syntax,
    ///        e.g. "/users/:id").
    /// @param handler Callback invoked on a match.
    Route(HTTP::HttpMethod method, std::string pattern, RouteHandler handler);
};

} // namespace FalconHTTP::Routing