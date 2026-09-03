/**
 * @file            Router.h
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
#include <FalconHTTP/Routing/Route.h>      // Route
#include <FalconHTTP/HTTP/HttpRequest.h>  // HttpRequest
#include <FalconHTTP/HTTP/HttpResponse.h> // HttpResponse

#include <VectorPro/Vector.h> // Vector
// clang-format on

using namespace VectorPro;

// Registers routes and dispatches requests to their handlers, matching a
// request's method and path against Route entries in registration order.

namespace FalconHTTP::Routing {

// Identity

/// Outcome of Router::dispatch().
enum class DispatchResult {
    /// A route matched both method and path; its handler was invoked.
    Matched,
    /// No registered route's pattern matched the request path
    /// (regardless of method). Server responds 404.
    NotFound,
    /// At least one registered route's pattern matched the request
    /// path, but none of those matches were for the request's
    /// method. Server responds 405. Note: if a path matches routes
    /// for several methods but not the requested one, this is
    /// returned without indicating *which* methods are allowed - the
    /// response currently carries no `Allow` header.
    MethodNotAllowed
};

/**
 * @class Router
 * @brief Holds the registered routes and matches incoming requests
 *        against them in registration order.
 *
 * @details
 * Matching is a linear scan over routes (first-match-wins per
 * method), not a trie/radix structure - fine for small route tables,
 * O(n) for large ones.
 */
class Router {
  public:
    // Identity

    /// Registered routes, in registration order. Public and directly
    /// mutable - callers can bypass get()/post()/put()/del() and
    /// append/modify entries here directly, which skips no validation
    /// (addRoute() currently does none), but does bypass the
    /// convenience of the typed registration methods.
    Vector<Route> routes;

    // Constructors
    Router() noexcept = default;

    // Registration

    /// Registers a GET handler for @p pattern (see PathMatcher for
    /// pattern syntax).
    void get(std::string pattern, RouteHandler handler);

    /// Registers a POST handler for @p pattern.
    void post(std::string pattern, RouteHandler handler);

    /// Registers a PUT handler for @p pattern.
    void put(std::string pattern, RouteHandler handler);

    /// Registers a DELETE handler for @p pattern. Named `del` rather
    /// than `delete` since `delete` is a reserved keyword in C++.
    void del(std::string pattern, RouteHandler handler);

    /// Registers a streaming (Stream-kind) handler for @p pattern,
    /// always under GET - SSE is a GET-only mechanism per spec, so
    /// there is no method parameter (unlike get()/post()/put()/del()).
    void stream(std::string pattern, StreamHandler handler);

    // Dispatch

    /**
     * @brief Finds the first registered Normal-kind route matching
     *        both @p request's method and path, invokes its handler,
     *        and reports the outcome.
     * @details Stream-kind routes are skipped entirely by this scan -
     *          dispatch() cannot invoke a StreamHandler (it has no
     *          SseConnection to give it, only an HttpResponse&), so a
     *          request matching only a Stream-kind route is reported
     *          as DispatchResult::NotFound here. Callers that want to
     *          handle streaming routes must call matchStream() first
     *          (see its docs) and only fall back to dispatch() when it
     *          returns nullptr - see Server::handleConnection().
     * @param request Path params are populated on a match (see
     *        HttpRequest::setPathParam()) before the handler runs.
     * @param response Populated by the matched handler; untouched if
     *        no route matched (caller - see Server::runChain() - is
     *        responsible for setting the 404/405 status/body based on
     *        the returned DispatchResult).
     * @return See DispatchResult.
     */
    [[nodiscard]] DispatchResult dispatch(HTTP::HttpRequest& request,
                                          HTTP::HttpResponse& response) const;

    /**
     * @brief Finds the first registered Stream-kind route matching
     *        both @p request's method and path.
     * @details Mirrors dispatch()'s matching logic but scans only
     *          Stream-kind routes and does not invoke anything - the
     *          caller (Server) invokes the returned handler itself
     *          once it has upgraded the connection to an
     *          SseConnection. Populates path params on a match, same
     *          as dispatch().
     * @param request Path params are populated on a match.
     * @return Pointer to the matched route's StreamHandler, or nullptr
     *         if no Stream-kind route matches @p request's method and
     *         path. The pointer is valid as long as this Router and
     *         its routes entry are not mutated/destroyed.
     */
    [[nodiscard]] const StreamHandler* matchStream(HTTP::HttpRequest& request) const;

  private:
    // Private Helpers

    /// Appends a Route(method, pattern, handler) to routes. No
    /// validation of @p pattern or duplicate-route detection is
    /// performed.
    void addRoute(HTTP::HttpMethod method, std::string pattern, RouteHandler handler);
};

} // namespace FalconHTTP::Routing
