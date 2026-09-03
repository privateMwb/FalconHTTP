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
#include <FalconHTTP/HTTP/HttpMethod.h>        // HttpMethod
#include <FalconHTTP/Routing/RouteHandler.h>  // RouteHandler
#include <FalconHTTP/Routing/StreamHandler.h> // StreamHandler

#include <string> // std::string
// clang-format on

// A single registered (method, pattern, handler) binding held in
// Router::routes.

namespace FalconHTTP::Routing {

/**
 * @enum RouteKind
 * @brief Which of Route's two handler members is live for a given entry.
 */
enum class RouteKind {
    /// handler is live; streamHandler is empty. Dispatched via
    /// Router::dispatch() into the normal one-shot response flow.
    Normal,
    /// streamHandler is live; handler is empty. Dispatched via
    /// Router::matchStream() - never invoked by Router::dispatch(),
    /// which skips Stream-kind routes entirely (see dispatch()'s docs).
    Stream
};

/**
 * @class Route
 * @brief One entry in Router::routes: which method and path pattern
 *        invoke which handler.
 *
 * @details
 * Exactly one of handler/streamHandler is populated, indicated by
 * kind - a Route is never both. Kept as two separate members rather
 * than a variant/union to match this codebase's existing style
 * (RouteHandler and StreamHandler are both FunctionPro::Function
 * instantiations with no common base).
 */
class Route {
  public:
    // Identity
    HTTP::HttpMethod method = HTTP::HttpMethod::Unknown;
    std::string pattern;
    RouteKind kind = RouteKind::Normal;
    RouteHandler handler;
    StreamHandler streamHandler;

    // Constructors

    /// Constructs an empty/unbound Route (method Unknown, empty
    /// pattern, kind Normal, empty handlers).
    Route() noexcept = default;

    /// Constructs a Normal-kind Route.
    /// @param method HTTP method this route responds to.
    /// @param pattern Path pattern (see PathMatcher for syntax,
    ///        e.g. "/users/:id").
    /// @param handler Callback invoked on a match.
    Route(HTTP::HttpMethod method, std::string pattern, RouteHandler handler);

    /// Constructs a Stream-kind Route.
    /// @param method HTTP method this route responds to.
    /// @param pattern Path pattern (see PathMatcher for syntax).
    /// @param handler Callback invoked on a match, given an
    ///        SseConnection instead of an HttpResponse.
    Route(HTTP::HttpMethod method, std::string pattern, StreamHandler handler);
};

} // namespace FalconHTTP::Routing